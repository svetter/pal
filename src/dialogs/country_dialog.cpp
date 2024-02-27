/*
 * Copyright 2023-2024 Simon Vetter
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
#include "src/settings/settings.h"

#include <QMessageBox>

using std::unique_ptr, std::make_unique;



/**
 * Creates a new country dialog.
 * 
 * Sets up the UI, restores geometry, connects interactive UI elements, and performs purpose-
 * specific preparations.
 * 
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param init			The country data to initialize the dialog with and store as initial data. CountryDialog takes ownership of this pointer.
 */
CountryDialog::CountryDialog(QWidget* parent, QMainWindow* mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Country> init) :
	ItemDialog(parent, mainWindow, db, purpose),
	init(std::move(init))
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,	{ nameLineEdit }}
	});
	
	setWindowIcon(QIcon(":/icons/ico/country_multisize_square.ico"));
	
	restoreDialogGeometry(this, mainWindow, &Settings::countryDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&CountryDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&CountryDialog::handle_cancel);
	
	
	changeUIForPurpose();
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		insertInitData();
		break;
	case multiEdit:
		// TODO
		break;
	default:
		assert(false);
	}
}

/**
 * Destroys the country dialog.
 */
CountryDialog::~CountryDialog()
{}



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
unique_ptr<Country> CountryDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	
	return make_unique<Country>(ItemID(), name);
}


/**
 * Checks whether changes have been made to the country, compared to the initial country object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool CountryDialog::changesMade()
{
	return !extractData()->equalTo(*init);
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
	const ValueColumn& nameColumn = db.countriesTable.nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void CountryDialog::aboutToClose()
{
	saveDialogGeometry(this, mainWindow, &Settings::countryDialog_geometry);
}





/**
 * Opens a new country dialog and saves the new country to the database.
 *
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @return				The index of the new country in the database's country table buffer.
 */
BufferRowIndex openNewCountryDialogAndStore(QWidget* parent, QMainWindow* mainWindow, Database& db)
{
	return openCountryDialogAndStore(parent, mainWindow, db, newItem, nullptr);
}

/**
 * Opens an edit country dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the country to edit in the database's country table buffer.
 * @return					True if any changes were made, false otherwise.
 */
bool openEditCountryDialogAndStore(QWidget* parent, QMainWindow* mainWindow, Database& db, BufferRowIndex bufferRowIndex)
{
	unique_ptr<Country> originalCountry = db.getCountryAt(bufferRowIndex);
	BufferRowIndex editedIndex = openCountryDialogAndStore(parent, mainWindow, db, editItem, std::move(originalCountry));
	return editedIndex.isValid();
}

/**
 * Opens a delete country dialog and deletes the country from the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the countries to delete in the database's country table buffer.
 * @return					True if any items were deleted, false otherwise.
 */
bool openDeleteCountriesDialogAndExecute(QWidget* parent, QMainWindow* mainWindow, Database& db, QSet<BufferRowIndex> bufferRowIndices)
{
	Q_UNUSED(mainWindow);
	if (bufferRowIndices.isEmpty()) return false;
	
	QSet<ValidItemID> countryIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		countryIDs += VALID_ITEM_ID(db.countriesTable.primaryKeyColumn.getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db.whatIf_removeRows(db.countriesTable, countryIDs);
	
	if (Settings::confirmDelete.get()) {
		bool plural = countryIDs.size() > 1;
		QString windowTitle = plural ? CountryDialog::tr("Delete countries") : CountryDialog::tr("Delete country");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}
	
	db.removeRows(parent, db.countriesTable, countryIDs);
	return true;
}



/**
 * Opens a purpose-generic country dialog and applies the resulting changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param originalCountry	The country data to initialize the dialog with and store as initial data. CountryDialog takes ownership of this pointer.
 * @return					The index of the new country in the database's country table buffer, or existing index of edited country. Invalid if the dialog was cancelled.
 */
BufferRowIndex openCountryDialogAndStore(QWidget* parent, QMainWindow* mainWindow, Database& db, DialogPurpose purpose, unique_ptr<Country> originalCountry)
{
	assert((bool) originalCountry != (purpose == newItem));
	
	const ItemID originalCountryID = (purpose != newItem) ? originalCountry->countryID : ItemID();
	if (purpose == duplicateItem) {
		originalCountry->countryID = ItemID();
	}
	BufferRowIndex newCountryIndex = BufferRowIndex();
	
	CountryDialog dialog = CountryDialog(parent, mainWindow, db, purpose, std::move(originalCountry));
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		unique_ptr<Country> extractedCountry = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newCountryIndex = db.countriesTable.addRow(parent, *extractedCountry);
			break;
		case editItem:
			db.countriesTable.updateRow(parent, FORCE_VALID(originalCountryID), *extractedCountry);
			
			// Set result to existing buffer row to signal that changes were made
			newCountryIndex = db.countriesTable.getBufferIndexForPrimaryKey(FORCE_VALID(originalCountryID));
			break;
		default:
			assert(false);
		}
	}
	
	return newCountryIndex;
}
