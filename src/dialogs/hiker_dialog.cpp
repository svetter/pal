#include "hiker_dialog.h"

#include <QMessageBox>



HikerDialog::HikerDialog(QWidget* parent, Hiker* init) :
		NewOrEditDialog(parent, tr("hiker"), init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,		this,	&HikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&HikerDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}



void HikerDialog::insertInitData()
{
	if (!init) return;
	// TODO
}



bool HikerDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())	return true;
	return false;
}



void HikerDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new hiker");
		QString message = tr("The hiker needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Hiker* openNewHikerDialogAndStore(QWidget* parent)
{
	HikerDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Hiker* hiker = new Hiker();
		hiker->name = dialog.nameTextbox->text();
		return hiker;
	}
	return nullptr;
}

bool openEditHikerDialog(QWidget* parent, Hiker* hiker)
{
	HikerDialog dialog(parent, hiker);
	dialog.exec();
	// TODO
	return false;
}
