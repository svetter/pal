#include "new_hiker.h"

#include <iostream>
#include <QMessageBox>



NewHikerDialog::NewHikerDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(okButton,		&QPushButton::clicked,		this,	&NewHikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&NewHikerDialog::handle_cancel);
}



bool NewHikerDialog::anyChanges()
{
	if (!nameTextbox->text().isEmpty())	return true;
	return false;
}



void NewHikerDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new hiker");
		QString question = tr("The hiker needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, question, ok, ok);
	}
}

void NewHikerDialog::handle_cancel()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new hiker");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewHikerDialog::reject()
{
	handle_cancel();
}



Hiker* openNewHikerDialogAndStore(QWidget *parent)
{
	NewHikerDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Hiker* hiker = new Hiker();
		hiker->name = dialog.nameTextbox->text();
		return hiker;
	}
	return nullptr;
}

bool openEditHikerDialog(QWidget *parent, Hiker* hiker)
{
	// TODO
	return false;
}
