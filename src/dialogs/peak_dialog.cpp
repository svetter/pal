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
 * @file peak_dialog.cpp
 * 
 * This file defines the PeakDialog class.
 */

#include "peak_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/region_dialog.h"
#include "src/settings/settings.h"

#include <QMessageBox>



/**
 * Creates a new peak dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, sets
 * initial values, and performs purpose-specific preparations.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 * @param init		The peak data to initialize the dialog with and store as initial data. PeakDialog takes ownership of this pointer.
 */
PeakDialog::PeakDialog(QWidget* parent, Database* db, DialogPurpose purpose, Peak* init) :
	ItemDialog(parent, db, purpose),
	init(init),
	selectableRegionIDs(QList<ValidItemID>())
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/peak_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::peakDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(heightCheckbox,		&QCheckBox::stateChanged,	this,	&PeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,	&QPushButton::clicked,		this,	&PeakDialog::handle_newRegion);
	
	connect(okButton,			&QPushButton::clicked,		this,	&PeakDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,		this,	&PeakDialog::handle_cancel);
	
	
	// Set initial height
	heightCheckbox->setChecked(Settings::peakDialog_heightEnabledInitially.get());
	handle_heightSpecifiedChanged();
	heightSpinner->setValue(Settings::peakDialog_initialHeight.get());
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	case duplicateItem:
		Peak* blankPeak = extractData();
		insertInitData();
		this->init = blankPeak;
		break;
	}
}

/**
 * Destroys the peak dialog.
 */
PeakDialog::~PeakDialog()
{
	delete init;
}



/**
 * Returns the window title to use when the dialog is used to edit an item.
 *
 * @return	The window title for editing an item
 */
QString PeakDialog::getEditWindowTitle()
{
	return tr("Edit peak");
}



/**
 * Populates the dialog's combo boxes with data from the database.
 */
void PeakDialog::populateComboBoxes()
{
	populateItemCombo(db->regionsTable, db->regionsTable->nameColumn, true, regionCombo, selectableRegionIDs);
}



/**
 * Inserts the data from the initial peak object into the dialog's UI elements.
 */
void PeakDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Height
	bool heightSpecified = init->heightSpecified();
	heightCheckbox->setChecked(heightSpecified);
	if (heightSpecified) {
		heightSpinner->setValue(init->height);
	}	
	handle_heightSpecifiedChanged();
	// Volcano
	volcanoCheckbox->setChecked(init->volcano);
	// Region
	if (init->regionID.isValid()) {
		regionCombo->setCurrentIndex(selectableRegionIDs.indexOf(FORCE_VALID(init->regionID)) + 1);	// 0 is None
	} else {
		regionCombo->setCurrentIndex(0);
	}
	// Links
	googleMapsLineEdit->setText(init->mapsLink);
	googleEarthLineEdit->setText(init->earthLink);
	wikipediaLineEdit->setText(init->wikiLink);
}


/**
 * Extracts the data from the UI elements and returns it as a peak object.
 *
 * @return	The peak data as a peak object. The caller takes ownership of the object.
 */
Peak* PeakDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	int		height		= parseSpinner		(heightSpinner);
	bool	volcano		= parseCheckbox		(volcanoCheckbox);
	ItemID	regionID	= parseItemCombo	(regionCombo, selectableRegionIDs, ItemTypeRegion);
	QString	mapsLink	= parseLineEdit		(googleMapsLineEdit);
	QString	earthLink	= parseLineEdit		(googleEarthLineEdit);
	QString	wikiLink	= parseLineEdit		(wikipediaLineEdit);
	
	if (!heightCheckbox->isChecked())	height = -1;
	
	Peak* peak = new Peak(ItemID(ItemTypePeak), name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
	return peak;
}


/**
 * Checks whether changes have been made to the peak, compared to the initial peak object, if set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool PeakDialog::changesMade()
{
	Peak* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



/**
 * Event handler for changes in the height specified checkbox.
 *
 * Enables or disables the height spinner depending on the checkbox's state.
 */
void PeakDialog::handle_heightSpecifiedChanged()
{
	bool enabled = heightCheckbox->isChecked();
	heightSpinner->setEnabled(enabled);
}

/**
 * Event handler for the new region button.
 *
 * Opens a new region dialog, adds the new region to the region combo box and selects it.
 */
void PeakDialog::handle_newRegion()
{
	BufferRowIndex newRegionIndex = openNewRegionDialogAndStore(this, db);
	if (newRegionIndex.isInvalid()) return;
	
	populateItemCombo(db->regionsTable, db->regionsTable->nameColumn, true, regionCombo, selectableRegionIDs);
	ValidItemID regionID = db->regionsTable->getPrimaryKeyAt(newRegionIndex);
	regionCombo->setCurrentIndex(selectableRegionIDs.indexOf(regionID) + 1);	// 0 is None
}



/**
 * Event handler for the OK button.
 *
 * Checks whether the name is empty or a duplicate depending on settings, prepares the dialog for
 * closing and then accepts it.
 */
void PeakDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save peak");
	QString emptyNameWindowMessage	= tr("The peak needs a name.");
	const ValueColumn* nameColumn = db->peaksTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void PeakDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::peakDialog_geometry);
}





static BufferRowIndex openPeakDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Peak* originalPeak);

/**
 * Opens a new peak dialog and saves the new peak to the database.
 *
 * @param parent	The parent window.
 * @param db		The project database.
 * @return			The index of the new peak in the database's peak table buffer.
 */
BufferRowIndex openNewPeakDialogAndStore(QWidget* parent, Database* db)
{
	return openPeakDialogAndStore(parent, db, newItem, nullptr);
}

/**
 * Opens a duplicate peak dialog and saves the new peak to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the peak to duplicate in the database's peak table buffer.
 * @return					The index of the new peak in the database's peak table buffer.
 */
BufferRowIndex openDuplicatePeakDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Peak* originalPeak = db->getPeakAt(bufferRowIndex);
	return openPeakDialogAndStore(parent, db, duplicateItem, originalPeak);
}

/**
 * Opens an edit peak dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the peak to edit in the database's peak table buffer.
 */
void openEditPeakDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Peak* originalPeak = db->getPeakAt(bufferRowIndex);
	openPeakDialogAndStore(parent, db, editItem, originalPeak);
}

/**
 * Opens a delete peak dialog and deletes the peak from the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the peaks to delete in the database's peak table buffer.
 */
void openDeletePeaksDialogAndExecute(QWidget* parent, Database* db, QSet<BufferRowIndex> bufferRowIndices)
{
	if (bufferRowIndices.isEmpty()) return;
	
	QSet<ValidItemID> peakIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		peakIDs += VALID_ITEM_ID(db->peaksTable->primaryKeyColumn->getValueAt(bufferRowIndex), ItemTypePeak);
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRows(db->peaksTable, peakIDs);
	
	if (Settings::confirmDelete.get()) {
		bool plural = peakIDs.size() > 1;
		QString windowTitle = plural ? PeakDialog::tr("Delete peaks") : PeakDialog::tr("Delete peak");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRows(parent, db->peaksTable, peakIDs);
}



/**
 * Opens a purpose-generic peak dialog and applies the resulting changes to the database.
 *
 * @param parent		The parent window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param originalPeak	The peak data to initialize the dialog with and store as initial data. PeakDialog takes ownership of this pointer.
 * @return				The index of the new peak in the database's peak table buffer. Invalid if the dialog was canceled or the purpose was editItem.
 */
static BufferRowIndex openPeakDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Peak* originalPeak)
{
	BufferRowIndex newPeakIndex = BufferRowIndex();
	if (purpose == duplicateItem) {
		assert(originalPeak);
		originalPeak->peakID = ItemID(ItemTypePeak);
	}
	
	PeakDialog dialog(parent, db, purpose, originalPeak);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Peak* extractedPeak = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newPeakIndex = db->peaksTable->addRow(parent, extractedPeak);
			break;
		case editItem:
			db->peaksTable->updateRow(parent, FORCE_VALID(originalPeak->peakID), extractedPeak);
			break;
		default:
			assert(false);
		}
		
		delete extractedPeak;
	}
	
	return newPeakIndex;
}
