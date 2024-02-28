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
 * @param windowTitle	The title of the dialog window.
 * @param init			The country data to initialize the dialog with and store as initial data. CountryDialog takes ownership of this pointer.
 */
CountryDialog::CountryDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, const QString& windowTitle, unique_ptr<const Country> init) :
	ItemDialog(parent, mainWindow, db, purpose, windowTitle),
	init(std::move(init))
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,	{ nameLineEdit }}
	});
	
	setWindowIcon(QIcon(":/icons/ico/country_multisize_square.ico"));
	
	restoreDialogGeometry(*this, mainWindow, Settings::countryDialog_geometry);
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
	QString	name	= parseLineEdit	(*nameLineEdit);
	
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
	saveDialogGeometry(*this, mainWindow, Settings::countryDialog_geometry);
}





/**
 * Opens a new country dialog and saves the new country to the database.
 *
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @return				The index of the new country in the database's country table buffer.
 */
BufferRowIndex openNewCountryDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db)
{
	const QString windowTitle = CountryDialog::tr("New country");
	
	CountryDialog dialog = CountryDialog(parent, mainWindow, db, newItem, windowTitle, nullptr);
	if (dialog.exec() != QDialog::Accepted) {
		return BufferRowIndex();
	}
	
	unique_ptr<Country> extractedCountry = dialog.extractData();
	
	const BufferRowIndex newCountryIndex = db.countriesTable.addRow(parent, *extractedCountry);
	return newCountryIndex;
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
bool openEditCountryDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex)
{
	unique_ptr<Country> originalCountry = db.getCountryAt(bufferRowIndex);
	const ItemID originalCountryID = originalCountry->countryID;
	
	const QString windowTitle = CountryDialog::tr("Edit country");
	
	CountryDialog dialog = CountryDialog(parent, mainWindow, db, editItem, windowTitle, std::move(originalCountry));
	if (dialog.exec() != QDialog::Accepted || !dialog.changesMade()) {
		return false;
	}
	
	unique_ptr<Country> extractedCountry = dialog.extractData();
	
	db.countriesTable.updateRow(parent, FORCE_VALID(originalCountryID), *extractedCountry);
	return true;
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
bool openDeleteCountriesDialogAndExecute(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices)
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
