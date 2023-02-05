#include "country_dialog.h"

#include <QMessageBox>



CountryDialog::CountryDialog(QWidget* parent, Database* db, Country* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&CountryDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&CountryDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}



void CountryDialog::insertInitData()
{
	if (!init) return;
	nameTextbox->setText(init->name);
}



bool CountryDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())	return true;
	return false;
}



void CountryDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save country");
		QString message = tr("The country needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Country* openNewCountryDialogAndStore(QWidget* parent, Database* db)
{
	CountryDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		QString	name = dialog.nameTextbox->text();
		Country* country = new Country(-1, name);
		// TODO
		return country;
	}
	return nullptr;
}

bool openEditCountryDialog(QWidget* parent, Database* db, Country* country)
{
	CountryDialog dialog(parent, db, country);
	dialog.exec();
	// TODO
	return false;
}
