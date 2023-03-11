#include "country_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



CountryDialog::CountryDialog(QWidget* parent, Database* db, DialogPurpose purpose, Country* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	
	restoreDialogGeometry(this, parent, &Settings::countryDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&CountryDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&CountryDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	default:
		assert(false);
	}
}

CountryDialog::~CountryDialog()
{
	delete init;
}



QString CountryDialog::getEditWindowTitle()
{
	return tr("Edit country");
}



void CountryDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
}


Country* CountryDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	
	Country* country = new Country(ItemID(), name);
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
	aboutToClose();
	
	if (!nameLineEdit->text().isEmpty() || Settings::allowEmptyNames.get()) {
		accept();
	} else {
		QString title = tr("Can't save country");
		QString message = tr("The country needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

void CountryDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::countryDialog_geometry);
}





static int openCountryDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Country* originalCountry);

int openNewCountryDialogAndStore(QWidget* parent, Database* db)
{
	return openCountryDialogAndStore(parent, db, newItem, nullptr);
}

void openEditCountryDialogAndStore(QWidget* parent, Database* db, Country* originalCountry)
{
	openCountryDialogAndStore(parent, db, editItem, originalCountry);
}

void openDeleteCountryDialogAndExecute(QWidget* parent, Database* db, Country* country)
{
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->countriesTable, country->countryID.forceValid());
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = CountryDialog::tr("Delete country");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	db->removeRow(parent, db->countriesTable, country->countryID.forceValid());
}



static int openCountryDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Country* originalCountry)
{
	int newCountryIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalCountry);
		originalCountry->countryID = ItemID();
	}
	
	CountryDialog dialog(parent, db, purpose, originalCountry);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Country* extractedCountry = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newCountryIndex = db->countriesTable->addRow(parent, extractedCountry);
			break;
		case editItem:
			db->countriesTable->updateRow(parent, originalCountry->countryID.forceValid(), extractedCountry);
			break;
		default:
			assert(false);
		}
		
		delete extractedCountry;
	}
	
	return newCountryIndex;
}
