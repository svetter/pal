#include "new_country.h"

#include <QMessageBox>



NewCountryDialog::NewCountryDialog(QWidget* parent): NewOrEditDialog(parent, tr("country"))
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(okButton,		&QPushButton::clicked,	this,	&NewCountryDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&NewCountryDialog::handle_cancel);
}



bool NewCountryDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())	return true;
	return false;
}



void NewCountryDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new country");
		QString message = tr("The country needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Country* openNewCountryDialogAndStore(QWidget* parent)
{
	NewCountryDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Country* country = new Country();
		country->name = dialog.nameTextbox->text();
		// TODO
		return country;
	}
	return nullptr;
}

bool openEditCountryDialog(QWidget* parent, Country* country)
{
	// TODO
	return false;
}
