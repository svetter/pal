#include "hiker_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



HikerDialog::HikerDialog(QWidget* parent, Database* db, Hiker* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,		this,	&HikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&HikerDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}

HikerDialog::~HikerDialog()
{
	delete init;
}



void HikerDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	nameLineEdit->setText(init->name);
}


Hiker* HikerDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	Hiker* hiker = new Hiker(-1, name);
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
	if (!nameLineEdit->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save hiker");
		QString message = tr("The hiker needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



int openNewHikerDialogAndStore(QWidget* parent, Database* db)
{
	int newHikerIndex = -1;
	
	HikerDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		Hiker* newHiker = dialog.extractData();
		newHikerIndex = db->hikersTable->addRow(newHiker);
		delete newHiker;
	}
	
	return newHikerIndex;
}

Hiker* openEditHikerDialog(QWidget* parent, Database* db, Hiker* originalHiker)
{
	Hiker* editedHiker = nullptr;
	
	HikerDialog dialog(parent, db, originalHiker);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		editedHiker = dialog.extractData();
		// TODO update database
	}
	
	delete originalHiker;
	return editedHiker;
}
