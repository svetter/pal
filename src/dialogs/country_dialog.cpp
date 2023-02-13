#include "country_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



CountryDialog::CountryDialog(QWidget* parent, Database* db, Country* init) :
		NewOrEditDialog(parent, db, init != nullptr, tr("Edit country")),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&CountryDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&CountryDialog::handle_cancel);
	
	
	if (edit) {	
		changeStringsForEdit(okButton);
		insertInitData();
	} else {
		this->init = extractData();
	}
}

CountryDialog::~CountryDialog()
{
	delete init;
}



void CountryDialog::insertInitData()
{
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



int openNewCountryDialogAndStore(QWidget* parent, Database* db)
{
	int newCountryIndex = -1;
	
	CountryDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		Country* newCountry = dialog.extractData();
		newCountryIndex = db->countriesTable->addRow(newCountry);
		delete newCountry;
	}
	
	return newCountryIndex;
}

void openEditCountryDialogAndStore(QWidget* parent, Database* db, Country* originalCountry)
{
	CountryDialog dialog(parent, db, originalCountry);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		Country* editedCountry = dialog.extractData();
		// TODO update database
		delete editedCountry;
	}
}
