#include "add_hiker_dialog.h"

#include <QMessageBox>



AddHikerDialog::AddHikerDialog(QWidget* parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(okButton,		&QPushButton::clicked,	this,	&AddHikerDialog::handle_newHiker);
	
	connect(okButton,		&QPushButton::clicked,	this,	&AddHikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_cancel);
}



bool AddHikerDialog::changesMade()
{
	if (hikerCombo->currentIndex() > 0)	return true;
	return false;
}



void AddHikerDialog::handle_newHiker()
{
	openNewHikerDialogAndStore(this);
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
		QMessageBox::information(this, title, question, ok, ok);
	}
}

void AddHikerDialog::handle_cancel()
{
	reject();
}



int openAddHikerDialog(QWidget* parent)
{
	AddHikerDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		// TODO
		return -1;
	}
	return -1;
}
