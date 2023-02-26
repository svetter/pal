#include "hiker_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



HikerDialog::HikerDialog(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	
	const QRect savedGeometry = Settings::hikerDialog_geometry.get();
	if (!savedGeometry.isEmpty()) {
		setGeometry(savedGeometry);
	}
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,		this,	&HikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&HikerDialog::handle_cancel);
	
	
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

HikerDialog::~HikerDialog()
{
	delete init;
}



QString HikerDialog::getEditWindowTitle()
{
	return tr("Edit hiker");
}



void HikerDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
}


Hiker* HikerDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	
	Hiker* hiker = new Hiker(ItemID(), name);
	return hiker;
}


bool HikerDialog::changesMade()
{
	Hiker* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void HikerDialog::handle_ok()
{
	aboutToClose();
	
	if (!nameLineEdit->text().isEmpty() || Settings::allowEmptyNames.get()) {
		accept();
	} else {
		QString title = tr("Can't save hiker");
		QString message = tr("The hiker needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

void HikerDialog::aboutToClose()
{	
	Settings::hikerDialog_geometry.set(geometry());
}





static int openHikerDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* originalHiker);

int openNewHikerDialogAndStore(QWidget* parent, Database* db)
{
	return openHikerDialogAndStore(parent, db, newItem, nullptr);
}

void openEditHikerDialogAndStore(QWidget* parent, Database* db, Hiker* originalHiker)
{
	openHikerDialogAndStore(parent, db, editItem, originalHiker);
}

void openDeleteHikerDialogAndExecute(QWidget* parent, Database* db, Hiker* hiker)
{
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->hikersTable, hiker->hikerID.forceValid());
	
	if (Settings::showDeleteWarnings.get()) {
		QString windowTitle = HikerDialog::tr("Delete hiker");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	db->removeRow(parent, db->hikersTable, hiker->hikerID.forceValid());
}



static int openHikerDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* originalHiker)
{
	int newHikerIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalHiker);
		originalHiker->hikerID = ItemID();
	}
	
	HikerDialog dialog(parent, db, purpose, originalHiker);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Hiker* extractedHiker = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newHikerIndex = db->hikersTable->addRow(parent, extractedHiker);
			break;
		case editItem:
			db->hikersTable->updateRow(parent, originalHiker->hikerID.forceValid(), extractedHiker);
			break;
		default:
			assert(false);
		}
		
		delete extractedHiker;
	}
	
	return newHikerIndex;
}
