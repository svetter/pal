#include "new_hiker.h"

#include <iostream>
#include <QMessageBox>



NewHikerDialog::NewHikerDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(okButton,		&QPushButton::clicked,		this,	&QDialog::accept);
	connect(cancelButton,	&QPushButton::clicked,		this,	&NewHikerDialog::handle_close);
}



bool NewHikerDialog::anyChanges()
{
	if (!nameTextbox->text().isEmpty())	return true;
	return false;
}



void NewHikerDialog::handle_close()
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
	handle_close();
}



Hiker* openNewHikerDialog(QWidget *parent)
{
	NewHikerDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		QString name = dialog.nameTextbox->text();
		return new Hiker(name);
	}
	return nullptr;
}

bool openEditHikerDialog(QWidget *parent, Hiker* hiker)
{
	// TODO
	return false;
}
