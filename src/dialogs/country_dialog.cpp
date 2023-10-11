/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

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
	QString emptyNameWindowTitle	= tr("Can't save country");
	QString emptyNameWindowMessage	= tr("The country needs a name.");
	const Column* nameColumn = db->countriesTable->nameColumn;
	NewOrEditDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
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

void openEditCountryDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Country* originalCountry = db->getCountryAt(bufferRowIndex);
	openCountryDialogAndStore(parent, db, editItem, originalCountry);
}

void openDeleteCountryDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex)
{
	Country* country = db->getCountryAt(bufferRowIndex);
	ValidItemID countryID = country->countryID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->countriesTable, countryID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = CountryDialog::tr("Delete country");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	db->removeRow(parent, db->countriesTable, countryID);
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
