#include "country_dialog.h"
#include "src/dialogs/parse_helper.h"

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

CountryDialog::~CountryDialog()
{
	delete init;
}



void CountryDialog::insertInitData()
{
	if (!init) return;
	nameLineEdit->setText(init->name);
}


Country* CountryDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	Country* country = new Country(-1, name);
	return country;
}


bool CountryDialog::changesMade()
{
	if (!nameLineEdit->text().isEmpty())	return true;
	return false;
}



void CountryDialog::handle_ok()
{
	if (!nameLineEdit->text().isEmpty()) {
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
		QString	name = dialog.nameLineEdit->text();
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
