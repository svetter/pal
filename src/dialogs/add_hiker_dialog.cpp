#include "add_hiker_dialog.h"
#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



AddHikerDialog::AddHikerDialog(QWidget* parent, Database* db) :
		QDialog(parent),
		db(db)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(newHikerButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_newHiker);
	
	connect(okButton,		&QPushButton::clicked,	this,	&AddHikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_cancel);
}



void AddHikerDialog::populateComboBoxes()
{
	// TODO #96 hikerCombo
}



int AddHikerDialog::extractHikerID()
{
	int	hikerID = parseIDCombo(hikerCombo);
	return hikerID;
}


bool AddHikerDialog::changesMade()
{
	return extractHikerID() > 0;
}



void AddHikerDialog::handle_newHiker()
{
	Hiker* newHiker = openNewHikerDialogAndStore(this, db);
	int hikerID = newHiker->hikerID;
	QString& name = newHiker->name;
	// TODO #96 add to hikerCombo
}


void AddHikerDialog::handle_ok()
{
	if (changesMade()) {
		accept();
	} else {
		QString title = tr("Can't add hiker");
		QString message = tr("You have to select a hiker.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

void AddHikerDialog::handle_cancel()
{
	reject();
}



int openAddHikerDialog(QWidget* parent, Database* db)
{
	AddHikerDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		return dialog.extractHikerID();
	}
	return -1;
}
