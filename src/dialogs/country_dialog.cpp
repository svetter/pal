/*
 * Copyright 2023-2025 Simon Vetter
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
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param init				The country data to initialize the dialog with and store as initial data. CountryDialog takes ownership of this pointer.
 * @param numItemsToEdit	The number of items to edit, if the purpose is multi-edit.
 */
CountryDialog::CountryDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Country> init, int numItemsToEdit) :
	ItemDialog(parent, mainWindow, db, purpose),
	init(std::move(init))
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,	{{ nameLineEdit },	{ &db.countriesTable.nameColumn }}}
	});
	
	setWindowIcon(QIcon(":/icons/ico/country_multisize_square.ico"));
	switch (purpose) {
	case newItem:
	case duplicateItem:	setWindowTitle(tr("New country"));										break;
	case editItem:
	case multiEdit:		setWindowTitle(tr("Edit %Ln country/countries", "", numItemsToEdit));	break;
	}
	
	restoreDialogGeometry(*this, mainWindow, Settings::countryDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&CountryDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&CountryDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
	case multiEdit:
		insertInitData();
		break;
	default:
		assert(false);
	}
	changeUIForPurpose();
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
	if (purpose == multiEdit) {
		return anyMultiEditChanges();
	}
	
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
 * @param callWhenDone	The function to call after the dialog has closed.
 */
void openNewCountryDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, std::function<void (BufferRowIndex)> callWhenDone)
{
	CountryDialog* dialog = new CountryDialog(parent, mainWindow, db, newItem, nullptr);
	
	auto callWhenClosed = [=, &parent, &db]() {
		BufferRowIndex newCountryIndex = BufferRowIndex();
		
		if (dialog->result() == QDialog::Accepted) {
			unique_ptr<Country> extractedCountry = dialog->extractData();
			
			db.beginChangingData();
			newCountryIndex = db.countriesTable.addRow(parent, *extractedCountry);
			db.finishChangingData();
		}
		
		delete dialog;
		return callWhenDone(newCountryIndex);
	};
	CountryDialog::connect(dialog, &CountryDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens an edit country dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the country to edit in the database's country table buffer.
 * @param callWhenDone		The function to call after the dialog has closed.
 */
void openEditCountryDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (bool)> callWhenDone)
{
	unique_ptr<Country> originalCountry = db.getCountryAt(bufferRowIndex);
	const ItemID originalCountryID = originalCountry->countryID;
	
	CountryDialog* dialog = new CountryDialog(parent, mainWindow, db, editItem, std::move(originalCountry));
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Country> extractedCountry = dialog->extractData();
			
			db.beginChangingData();
			db.countriesTable.updateRow(parent, FORCE_VALID(originalCountryID), *extractedCountry);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	CountryDialog::connect(dialog, &CountryDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a multi-edit country dialog and saves the changes to the database.
 * 
 * @param parent				The parent window.
 * @param mainWindow			The application's main window.
 * @param db					The project database.
 * @param bufferRowIndices		The buffer row indices of the countries to edit.
 * @param initBufferRowIndex	The index of the country whose data to initialize the dialog with.
 * @param callWhenDone			The function to call after the dialog has closed.
 */
void openMultiEditCountriesDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices, BufferRowIndex initBufferRowIndex, std::function<void (bool)> callWhenDone)
{
	assert(!bufferRowIndices.isEmpty());
	
	unique_ptr<Country> originalCountry = db.getCountryAt(initBufferRowIndex);
	
	CountryDialog* dialog = new CountryDialog(parent, mainWindow, db, multiEdit, std::move(originalCountry), bufferRowIndices.size());
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Country> extractedCountry = dialog->extractData();
			extractedCountry->countryID = ItemID();
			QSet<const Column*> columnsToSave = dialog->getMultiEditColumns();
			QList<const Column*> columnList = QList<const Column*>(columnsToSave.constBegin(), columnsToSave.constEnd());
			
			db.beginChangingData();
			db.countriesTable.updateRows(parent, bufferRowIndices, columnList, *extractedCountry);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	CountryDialog::connect(dialog, &CountryDialog::finished, callWhenClosed);
	
	dialog->open();
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
		const QString windowTitle = CountryDialog::tr("Delete %Ln country/countries", "", countryIDs.size());
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}
	
	db.beginChangingData();
	db.removeRows(parent, db.countriesTable, countryIDs);
	db.finishChangingData();
	return true;
}
