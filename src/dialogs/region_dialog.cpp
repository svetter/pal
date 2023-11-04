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
 * @file region_dialog.cpp
 * 
 * This file defines the RegionDialog class.
 */

#include "region_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/main/settings.h"

#include <QMessageBox>



/**
 * Creates a new region dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, and
 * performs purpose-specific preparations.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 * @param init		The region data to initialize the dialog with and store as initial data. RegionDialog takes ownership of this pointer.
 */
RegionDialog::RegionDialog(QWidget* parent, Database* db, DialogPurpose purpose, Region* init) :
		ItemDialog(parent, db, purpose),
		init(init),
		selectableRangeIDs(QList<ValidItemID>()),
		selectableCountryIDs(QList<ValidItemID>())
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/region_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::regionDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(newRangeButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_newRange);
	connect(newCountryButton,	&QPushButton::clicked,	this,	&RegionDialog::handle_newCountry);
	
	connect(okButton,			&QPushButton::clicked,	this,	&RegionDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_cancel);
	
	
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
 * Destroys the region dialog.
 */
RegionDialog::~RegionDialog()
{
	delete init;
}



/**
 * Returns the window title to use when the dialog is used to edit an item.
 *
 * @return	The window title for editing an item
 */
QString RegionDialog::getEditWindowTitle()
{
	return tr("Edit region");
}



/**
 * Populates the dialog's combo boxes with data from the database.
 */
void RegionDialog::populateComboBoxes()
{
	populateItemCombo(db->rangesTable, db->rangesTable->nameColumn, true, rangeCombo, selectableRangeIDs);
	
	populateItemCombo(db->countriesTable, db->countriesTable->nameColumn, true, countryCombo, selectableCountryIDs);
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
		rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(init->rangeID.get()) + 1);	// 0 is None
	} else {
		rangeCombo->setCurrentIndex(0);
	}
	// Country
	if (init->countryID.isValid()) {
		countryCombo->setCurrentIndex(selectableCountryIDs.indexOf(init->countryID.get()) + 1);	// 0 is None
	} else {
		countryCombo->setCurrentIndex(0);
	}
}


/**
 * Extracts the data from the UI elements and returns it as a region object.
 *
 * @return	The region data as a region object. The caller takes ownership of the object.
 */
Region* RegionDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	ItemID	rangeID		= parseItemCombo	(rangeCombo, selectableRangeIDs);
	ItemID	countryID	= parseItemCombo	(countryCombo, selectableCountryIDs);
	
	Region* region = new Region(ItemID(), name, rangeID, countryID);
	return region;
}


/**
 * Checks whether changes have been made to the region, compared to the initial region object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool RegionDialog::changesMade()
{
	Region* currentState = extractData();
	bool equal = currentState == init;
	delete currentState;
	return !equal;
}



/**
 * Event handler for the new range button.
 *
 * Opens a new range dialog, adds the new range to the range combo box and selects it.
 */
void RegionDialog::handle_newRange()
{
	BufferRowIndex newRangeIndex = openNewRangeDialogAndStore(this, db);
	if (newRangeIndex.isInvalid()) return;
	
	populateItemCombo(db->rangesTable, db->rangesTable->nameColumn, true, rangeCombo, selectableRangeIDs);
	ValidItemID rangeID = db->rangesTable->getPrimaryKeyAt(newRangeIndex);
	rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(rangeID) + 1);	// 0 is None
}

/**
 * Event handler for the new country button.
 *
 * Opens a new country dialog, adds the new country to the country combo box and selects it.
 */
void RegionDialog::handle_newCountry()
{
	BufferRowIndex newCountryIndex = openNewCountryDialogAndStore(this, db);
	if (newCountryIndex.isInvalid()) return;
	
	populateItemCombo(db->countriesTable, db->countriesTable->nameColumn, true, countryCombo, selectableCountryIDs);
	ValidItemID countryID = db->countriesTable->getPrimaryKeyAt(newCountryIndex);
	countryCombo->setCurrentIndex(selectableCountryIDs.indexOf(countryID) + 1);	// 0 is None
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
	const Column* nameColumn = db->regionsTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void RegionDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::regionDialog_geometry);
}





static BufferRowIndex openRegionDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Region* originalRegion);

/**
 * Opens a new region dialog and saves the new region to the database.
 *
 * @param parent	The parent window.
 * @param db		The project database.
 * @return			The index of the new region in the database's region table buffer.
 */
BufferRowIndex openNewRegionDialogAndStore(QWidget* parent, Database* db)
{
	return openRegionDialogAndStore(parent, db, newItem, nullptr);
}

/**
 * Opens an edit region dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the region to edit in the database's region table buffer.
 */
void openEditRegionDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Region* originalRegion = db->getRegionAt(bufferRowIndex);
	openRegionDialogAndStore(parent, db, editItem, originalRegion);
}

/**
 * Opens a delete region dialog and deletes the region from the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the region to delete in the database's region table buffer.
 */
void openDeleteRegionDialogAndExecute(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Region* region = db->getRegionAt(bufferRowIndex);
	ValidItemID regionID = region->regionID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->regionsTable, regionID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = RegionDialog::tr("Delete region");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->regionsTable, regionID);
}



/**
 * Opens a purpose-generic region dialog and applies the resulting changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param originalRegion	The region data to initialize the dialog with and store as initial data. Region takes ownership of this pointer.
 * @return					The index of the new region in the database's region table buffer. Invalid if the dialog was canceled or the purpose was editItem.
 */
static BufferRowIndex openRegionDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Region* originalRegion)
{
	BufferRowIndex newRegionIndex = BufferRowIndex();
	if (purpose == duplicateItem) {
		assert(originalRegion);
		originalRegion->regionID = ItemID();
	}
	
	RegionDialog dialog(parent, db, purpose, originalRegion);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Region* extractedRegion = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newRegionIndex = db->regionsTable->addRow(parent, extractedRegion);
			break;
		case editItem:
			db->regionsTable->updateRow(parent, originalRegion->regionID.forceValid(), extractedRegion);
			break;
		default:
			assert(false);
		}
		
		delete extractedRegion;
	}
	
	return newRegionIndex;
}
