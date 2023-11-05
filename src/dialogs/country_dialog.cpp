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

/**
 * @file country_dialog.cpp
 * 
 * This file defines the CountryDialog class.
 */

#include "country_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



/**
 * Creates a new country dialog.
 * 
 * Sets up the UI, restores geometry, connects interactive UI elements, and performs purpose-
 * specific preparations.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 * @param init		The country data to initialize the dialog with and store as initial data. CountryDialog takes ownership of this pointer.
 */
CountryDialog::CountryDialog(QWidget* parent, Database* db, DialogPurpose purpose, Country* init) :
		ItemDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/country_multisize_square.ico"));
	
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

/**
 * Destroys the country dialog.
 */
CountryDialog::~CountryDialog()
{
	delete init;
}



/**
 * Returns the window title to use when the dialog is used to edit an item.
 *
 * @return	The window title for editing an item
 */
QString CountryDialog::getEditWindowTitle()
{
	return tr("Edit country");
}



/**
 * Inserts the data from the initial country object into the dialog's UI elements.
 */
void CountryDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
}


/**
 * Extracts the data from the UI elements and returns it as a country object.
 *
 * @return	The country data as a country object. The caller takes ownership of the object.
 */
Country* CountryDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	
	Country* country = new Country(ItemID(), name);
	return country;
}


/**
 * Checks whether changes have been made to the country, compared to the initial country object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool CountryDialog::changesMade()
{
	Country* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



/**
 * Event handler for the OK button.
 * 
 * Checks whether the name is empty or a duplicate depending on settings, prepares the dialog for
 * closing and then accepts it.
 */
void CountryDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save country");
	QString emptyNameWindowMessage	= tr("The country needs a name.");
	const Column* nameColumn = db->countriesTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void CountryDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::countryDialog_geometry);
}





static BufferRowIndex openCountryDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Country* originalCountry);

/**
 * Opens a new country dialog and saves the new country to the database.
 *
 * @param parent	The parent window.
 * @param db		The project database.
 * @return			The index of the new country in the database's country table buffer.
 */
BufferRowIndex openNewCountryDialogAndStore(QWidget* parent, Database* db)
{
	return openCountryDialogAndStore(parent, db, newItem, nullptr);
}

/**
 * Opens an edit country dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the country to edit in the database's country table buffer.
 */
void openEditCountryDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Country* originalCountry = db->getCountryAt(bufferRowIndex);
	openCountryDialogAndStore(parent, db, editItem, originalCountry);
}

/**
 * Opens a delete country dialog and deletes the country from the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the country to delete in the database's country table buffer.
 */
void openDeleteCountryDialogAndExecute(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Country* country = db->getCountryAt(bufferRowIndex);
	ValidItemID countryID = FORCE_VALID(country->countryID);
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->countriesTable, countryID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = CountryDialog::tr("Delete country");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	db->removeRow(parent, db->countriesTable, countryID);
}



/**
 * Opens a purpose-generic country dialog and applies the resulting changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param originalCountry	The country data to initialize the dialog with and store as initial data. CountryDialog takes ownership of this pointer.
 * @return					The index of the new country in the database's country table buffer. Invalid if the dialog was canceled or the purpose was editItem.
 */
static BufferRowIndex openCountryDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Country* originalCountry)
{
	BufferRowIndex newCountryIndex = BufferRowIndex();
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
			db->countriesTable->updateRow(parent, FORCE_VALID(originalCountry->countryID), extractedCountry);
			break;
		default:
			assert(false);
		}
		
		delete extractedCountry;
	}
	
	return newCountryIndex;
}
