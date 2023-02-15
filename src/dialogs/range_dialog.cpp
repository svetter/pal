#include "range_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



RangeDialog::RangeDialog(QWidget* parent, Database* db, Range* init) :
		NewOrEditDialog(parent, db, init != nullptr, tr("Edit mountain range")),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
	
	
	if (edit) {	
		changeStringsForEdit(okButton);
		insertInitData();
	} else {
		this->init = extractData();
	}
}

RangeDialog::~RangeDialog()
{
	delete init;
}



void RangeDialog::populateComboBoxes()
{
	continentCombo->insertItems(1, Range::continentNames);
}



void RangeDialog::insertInitData()
{
	nameLineEdit->setText(init->name);
	continentCombo->setCurrentIndex(init->continent);
}


Range* RangeDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	int		continent	= parseEnumCombo	(continentCombo, true);
	
	Range* range = new Range(-1, name, continent);
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
	if (!nameLineEdit->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save mountain range");
		QString message = tr("The mountain range needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



int openNewRangeDialogAndStore(QWidget* parent, Database* db)
{
	int newRangeIndex = -1;
	
	RangeDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		Range* newRange = dialog.extractData();
		newRangeIndex = db->rangesTable->addRow(parent, newRange);
		delete newRange;
	}
	
	return newRangeIndex;
}

void openEditRangeDialogAndStore(QWidget* parent, Database* db, Range* originalRange)
{
	RangeDialog dialog(parent, db, originalRange);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		Range* editedRange = dialog.extractData();
		// TODO update database	
		delete editedRange;
	}
}
