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
	
	connect(newHikerButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_newHiker);
	
	connect(okButton,		&QPushButton::clicked,	this,	&AddHikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_cancel);
}



int AddHikerDialog::extractHikerID()
{
	int	hikerID = parseIDCombo(hikerCombo);
	return hikerID;
}


bool AddHikerDialog::changesMade()
{
	if (hikerCombo->currentIndex() > 0)	return true;
	return false;
}



void AddHikerDialog::handle_newHiker()
{
	openNewHikerDialogAndStore(this, db);
	// TODO
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
		// TODO
		return -1;
	}
	return -1;
}
