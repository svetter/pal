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
	if (!edit) {
		init = extractData();
		return;
	}
	
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
	Country* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
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
	Country* newCountry = nullptr;
	
	CountryDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		newCountry = dialog.extractData();
		int countryID = db->countriesTable->addRow(newCountry);
		newCountry->countryID = countryID;
	}
	
	return newCountry;
}

Country* openEditCountryDialog(QWidget* parent, Database* db, Country* originalCountry)
{
	Country* editedCountry = nullptr;
	
	CountryDialog dialog(parent, db, originalCountry);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		editedCountry = dialog.extractData();
		// TODO update database
	}
	
	delete originalCountry;
	return editedCountry;
}
