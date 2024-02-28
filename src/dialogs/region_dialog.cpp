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
 * @file region_dialog.cpp
 * 
 * This file defines the RegionDialog class.
 */

#include "region_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/settings/settings.h"

#include <QMessageBox>

using std::unique_ptr, std::make_unique;



/**
 * Creates a new region dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, and
 * performs purpose-specific preparations.
 * 
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param windowTitle	The title of the dialog window.
 * @param init			The region data to initialize the dialog with and store as initial data. RegionDialog takes ownership of this pointer.
 */
RegionDialog::RegionDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, const QString& windowTitle, unique_ptr<const Region> init) :
	ItemDialog(parent, mainWindow, db, purpose, windowTitle),
	init(std::move(init)),
	selectableRangeIDs(QList<ValidItemID>()),
	selectableCountryIDs(QList<ValidItemID>())
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,		{{ nameLineEdit },						{ &db.regionsTable.nameColumn }}},
		{rangeCheckbox,		{{ rangeCombo, newRangeButton },		{ &db.regionsTable.rangeIDColumn }}},
		{countryCheckbox,	{{ countryCombo, newCountryButton },	{ &db.regionsTable.countryIDColumn }}}
	});
	
	setWindowIcon(QIcon(":/icons/ico/region_multisize_square.ico"));
	
	restoreDialogGeometry(*this, mainWindow, Settings::regionDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	populateComboBoxes();
	
	
	connect(newRangeButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_newRange);
	connect(newCountryButton,	&QPushButton::clicked,	this,	&RegionDialog::handle_newCountry);
	
	connect(okButton,			&QPushButton::clicked,	this,	&RegionDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_cancel);
	
	
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
 * Destroys the region dialog.
 */
RegionDialog::~RegionDialog()
{}



/**
 * Populates the dialog's combo boxes with data from the database.
 */
void RegionDialog::populateComboBoxes()
{
	populateRangeCombo(db, *rangeCombo, selectableRangeIDs);
	
	populateCountryCombo(db, *countryCombo, selectableCountryIDs);
}



/**
 * Inserts the data from the initial region object into the dialog's UI elements.
 */
void RegionDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Range
	if (init->rangeID.isValid()) {
		rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(FORCE_VALID(init->rangeID)) + 1);	// 0 is None
	} else {
		rangeCombo->setCurrentIndex(0);
	}
	// Country
	if (init->countryID.isValid()) {
		countryCombo->setCurrentIndex(selectableCountryIDs.indexOf(FORCE_VALID(init->countryID)) + 1);	// 0 is None
	} else {
		countryCombo->setCurrentIndex(0);
	}
}


/**
 * Extracts the data from the UI elements and returns it as a region object.
 *
 * @return	The region data as a region object. The caller takes ownership of the object.
 */
unique_ptr<Region> RegionDialog::extractData()
{
	QString	name		= parseLineEdit		(*nameLineEdit);
	ItemID	rangeID		= parseItemCombo	(*rangeCombo, selectableRangeIDs);
	ItemID	countryID	= parseItemCombo	(*countryCombo, selectableCountryIDs);
	
	return make_unique<Region>(ItemID(), name, rangeID, countryID);
}


/**
 * Checks whether changes have been made to the region, compared to the initial region object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool RegionDialog::changesMade()
{
	if (purpose == multiEdit) {
		return anyMultiEditChanges();
	}
	
	return !extractData()->equalTo(*init);
}



/**
 * Event handler for the new range button.
 *
 * Opens a new range dialog, adds the new range to the range combo box and selects it.
 */
void RegionDialog::handle_newRange()
{
	BufferRowIndex newRangeIndex = openNewRangeDialogAndStore(*this, mainWindow, db);
	if (newRangeIndex.isInvalid()) return;
	
	populateRangeCombo(db, *rangeCombo, selectableRangeIDs);
	const ValidItemID newRangeID = db.rangesTable.getPrimaryKeyAt(newRangeIndex);
	rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(newRangeID) + 1);	// 0 is None
}

/**
 * Event handler for the new country button.
 *
 * Opens a new country dialog, adds the new country to the country combo box and selects it.
 */
void RegionDialog::handle_newCountry()
{
	BufferRowIndex newCountryIndex = openNewCountryDialogAndStore(*this, mainWindow, db);
	if (newCountryIndex.isInvalid()) return;
	
	populateCountryCombo(db, *countryCombo, selectableCountryIDs);
	const ValidItemID newCountryID = db.countriesTable.getPrimaryKeyAt(newCountryIndex);
	countryCombo->setCurrentIndex(selectableCountryIDs.indexOf(newCountryID) + 1);	// 0 is None
}



/**
 * Event handler for the OK button.
 *
 * Checks whether the name is empty or a duplicate depending on settings, prepares the dialog for
 * closing and then accepts it.
 */
void RegionDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save region");
	QString emptyNameWindowMessage	= tr("The region needs a name.");
	const ValueColumn& nameColumn = db.regionsTable.nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void RegionDialog::aboutToClose()
{
	saveDialogGeometry(*this, mainWindow, Settings::regionDialog_geometry);
}





/**
 * Opens a new region dialog and saves the new region to the database.
 *
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @return				The index of the new region in the database's region table buffer.
 */
BufferRowIndex openNewRegionDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db)
{
	const QString windowTitle = RegionDialog::tr("New region");
	
	RegionDialog dialog = RegionDialog(parent, mainWindow, db, newItem, windowTitle, nullptr);
	if (dialog.exec() != QDialog::Accepted) {
		return BufferRowIndex();
	}
	
	unique_ptr<Region> extractedRegion = dialog.extractData();
	
	const BufferRowIndex newRegionIndex = db.regionsTable.addRow(parent, *extractedRegion);
	return newRegionIndex;
}

/**
 * Opens an edit region dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the region to edit in the database's region table buffer.
 * @return					True if any changes were made, false otherwise.
 */
bool openEditRegionDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex)
{
	unique_ptr<Region> originalRegion = db.getRegionAt(bufferRowIndex);
	const ItemID originalRegionID = originalRegion->regionID;
	
	const QString windowTitle = RegionDialog::tr("Edit region");
	
	RegionDialog dialog = RegionDialog(parent, mainWindow, db, editItem, windowTitle, std::move(originalRegion));
	if (dialog.exec() != QDialog::Accepted || !dialog.changesMade()) {
		return false;
	}
	
	unique_ptr<Region> extractedRegion = dialog.extractData();
	
	db.regionsTable.updateRow(parent, FORCE_VALID(originalRegionID), *extractedRegion);
	return true;
}

/**
 * Opens a delete region dialog and deletes the region from the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the regions to delete in the database's region table buffer.
 * @return					True if any items were deleted, false otherwise.
 */
bool openDeleteRegionsDialogAndExecute(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices)
{
	Q_UNUSED(mainWindow);
	if (bufferRowIndices.isEmpty()) return false;
	
	QSet<ValidItemID> regionIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		regionIDs += VALID_ITEM_ID(db.regionsTable.primaryKeyColumn.getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db.whatIf_removeRows(db.regionsTable, regionIDs);
	
	if (Settings::confirmDelete.get()) {
		bool plural = regionIDs.size() > 1;
		QString windowTitle = plural ? RegionDialog::tr("Delete regions") : RegionDialog::tr("Delete region");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}

	db.removeRows(parent, db.regionsTable, regionIDs);
	return true;
}
