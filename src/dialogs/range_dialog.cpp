#include "range_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



RangeDialog::RangeDialog(QWidget* parent, Database* db, DialogPurpose purpose, Range* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	
	const QRect savedGeometry = Settings::rangeDialog_geometry.get();
	if (!savedGeometry.isEmpty()) {
		setGeometry(savedGeometry);
	}
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	default:
		assert(false);
	}
}

RangeDialog::~RangeDialog()
{
	delete init;
}



QString RangeDialog::getEditWindowTitle()
{
	return tr("Edit mountain range");
}



void RangeDialog::populateComboBoxes()
{
	continentCombo->insertItems(1, Range::continentNames);
}



void RangeDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Continent
	continentCombo->setCurrentIndex(init->continent);
}


Range* RangeDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	int		continent	= parseEnumCombo	(continentCombo, true);
	
	Range* range = new Range(ItemID(), name, continent);
	return range;
}


bool RangeDialog::changesMade()
{
	Range* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void RangeDialog::handle_ok()
{
	aboutToClose();
	
	if (!nameLineEdit->text().isEmpty() || Settings::allowEmptyNames.get()) {
		accept();
	} else {
		QString title = tr("Can't save mountain range");
		QString message = tr("The mountain range needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

void RangeDialog::aboutToClose()
{	
	Settings::rangeDialog_geometry.set(geometry());
}





static int openRangeDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Range* originalRange);

int openNewRangeDialogAndStore(QWidget* parent, Database* db)
{
	return openRangeDialogAndStore(parent, db, newItem, nullptr);
}

void openEditRangeDialogAndStore(QWidget* parent, Database* db, Range* originalRange)
{
	openRangeDialogAndStore(parent, db, editItem, originalRange);
}

void openDeleteRangeDialogAndExecute(QWidget* parent, Database* db, Range* range)
{
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->rangesTable, range->rangeID.forceValid());
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = RangeDialog::tr("Delete mountain range");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->rangesTable, range->rangeID.forceValid());
}



static int openRangeDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Range* originalRange)
{
	int newRangeIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalRange);
		originalRange->rangeID = ItemID();
	}
	
	RangeDialog dialog(parent, db, purpose, originalRange);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Range* extractedRange = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newRangeIndex = db->rangesTable->addRow(parent, extractedRange);
			break;
		case editItem:
			db->rangesTable->updateRow(parent, originalRange->rangeID.forceValid(), extractedRange);
			break;
		default:
			assert(false);
		}
		
		delete extractedRange;
	}
	
	return newRangeIndex;
}
