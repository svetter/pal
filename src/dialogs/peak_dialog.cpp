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
 * @file peak_dialog.cpp
 * 
 * This file defines the PeakDialog class.
 */

#include "peak_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/region_dialog.h"
#include "src/settings/settings.h"

#include <QMessageBox>

using std::unique_ptr, std::make_unique;



/**
 * Creates a new peak dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, sets
 * initial values, and performs purpose-specific preparations.
 * 
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param windowTitle	The title of the dialog window.
 * @param init			The peak data to initialize the dialog with and store as initial data. PeakDialog takes ownership of this pointer.
 */
PeakDialog::PeakDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, const QString& windowTitle, unique_ptr<const Peak> init) :
	ItemDialog(parent, mainWindow, db, purpose, windowTitle),
	init(std::move(init)),
	selectableRegionIDs(QList<ValidItemID>())
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,		{{ nameLineEdit },							{ &db.peaksTable.nameColumn }}},
		{regionCheckbox,	{{ regionCombo, newRegionButton },			{ &db.peaksTable.regionIDColumn }}},
		{heightCheckbox,	{{ heightSpecifyCheckbox, heightSpinner },	{ &db.peaksTable.heightColumn }}},
		{mapsCheckbox,		{{ mapsLineEdit },							{ &db.peaksTable.mapsLinkColumn }}},
		{earthCheckbox,		{{ earthLineEdit },							{ &db.peaksTable.earthLinkColumn }}},
		{wikipediaCheckbox,	{{ wikipediaLineEdit },						{ &db.peaksTable.wikiLinkColumn }}}
	}, {
		{volcanoCheckbox, { &db.peaksTable.volcanoColumn }}
	});
	
	setWindowIcon(QIcon(":/icons/ico/peak_multisize_square.ico"));
	
	restoreDialogGeometry(*this, mainWindow, Settings::peakDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	populateComboBoxes();
	
	
	connect(heightSpecifyCheckbox,	&QCheckBox::stateChanged,	this,	&PeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,		&QPushButton::clicked,		this,	&PeakDialog::handle_newRegion);
	
	connect(okButton,				&QPushButton::clicked,		this,	&PeakDialog::handle_ok);
	connect(cancelButton,			&QPushButton::clicked,		this,	&PeakDialog::handle_cancel);
	
	
	// Set initial height
	heightSpecifyCheckbox->setChecked(Settings::peakDialog_heightEnabledInitially.get());
	handle_heightSpecifiedChanged();
	heightSpinner->setValue(Settings::peakDialog_initialHeight.get());
	
	
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
	case duplicateItem:
		unique_ptr<Peak> blankPeak = extractData();
		insertInitData();
		this->init = std::move(blankPeak);
		break;
	}
}

/**
 * Destroys the peak dialog.
 */
PeakDialog::~PeakDialog()
{}



/**
 * Populates the dialog's combo boxes with data from the database.
 */
void PeakDialog::populateComboBoxes()
{
	populateRegionCombo(db, *regionCombo, selectableRegionIDs);
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
	heightSpecifyCheckbox->setChecked(heightSpecified);
	if (heightSpecified) {
		heightSpinner->setValue(init->height);
	}
	// Volcano
	volcanoCheckbox->setChecked(init->volcano);
	// Region
	if (init->regionID.isValid()) {
		regionCombo->setCurrentIndex(selectableRegionIDs.indexOf(FORCE_VALID(init->regionID)) + 1);	// 0 is None
	} else {
		regionCombo->setCurrentIndex(0);
	}
	// Links
	mapsLineEdit->setText(init->mapsLink);
	earthLineEdit->setText(init->earthLink);
	wikipediaLineEdit->setText(init->wikiLink);
}


/**
 * Extracts the data from the UI elements and returns it as a peak object.
 *
 * @return	The peak data as a peak object. The caller takes ownership of the object.
 */
unique_ptr<Peak> PeakDialog::extractData()
{
	QString	name		= parseLineEdit		(*nameLineEdit);
	int		height		= parseSpinner		(*heightSpinner);
	bool	volcano		= parseCheckbox		(*volcanoCheckbox);
	ItemID	regionID	= parseItemCombo	(*regionCombo, selectableRegionIDs);
	QString	mapsLink	= parseLineEdit		(*mapsLineEdit);
	QString	earthLink	= parseLineEdit		(*earthLineEdit);
	QString	wikiLink	= parseLineEdit		(*wikipediaLineEdit);
	
	if (!heightSpecifyCheckbox->isChecked()) height = -1;
	
	return make_unique<Peak>(ItemID(), name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
}


/**
 * Checks whether changes have been made to the peak, compared to the initial peak object, if set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool PeakDialog::changesMade()
{
	if (purpose == multiEdit) {
		return anyMultiEditChanges();
	}
	
	return !extractData()->equalTo(*init);
}



/**
 * Event handler for changes in the height specified checkbox.
 *
 * Enables or disables the height spinner depending on the checkbox's state.
 */
void PeakDialog::handle_heightSpecifiedChanged()
{
	bool enabled = heightSpecifyCheckbox->isChecked();
	heightSpinner->setEnabled(enabled);
}

/**
 * Event handler for the new region button.
 *
 * Opens a new region dialog, adds the new region to the region combo box and selects it.
 */
void PeakDialog::handle_newRegion()
{
	BufferRowIndex newRegionIndex = openNewRegionDialogAndStore(*this, mainWindow, db);
	if (newRegionIndex.isInvalid()) return;
	
	populateRegionCombo(db, *regionCombo, selectableRegionIDs);
	const ValidItemID newRegionID = db.regionsTable.getPrimaryKeyAt(newRegionIndex);
	regionCombo->setCurrentIndex(selectableRegionIDs.indexOf(newRegionID) + 1);	// 0 is None
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
	const ValueColumn& nameColumn = db.peaksTable.nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void PeakDialog::aboutToClose()
{
	saveDialogGeometry(*this, mainWindow, Settings::peakDialog_geometry);
}





/**
 * Opens a new peak dialog and saves the new peak to the database.
 *
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @return				The index of the new peak in the database's peak table buffer.
 */
BufferRowIndex openNewPeakDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db)
{
	const QString windowTitle = PeakDialog::tr("New peak");
	
	PeakDialog dialog = PeakDialog(parent, mainWindow, db, newItem, windowTitle, nullptr);
	if (dialog.exec() != QDialog::Accepted) {
		return BufferRowIndex();
	}
	
	unique_ptr<Peak> extractedPeak = dialog.extractData();
	
	const BufferRowIndex newPeakIndex = db.peaksTable.addRow(parent, *extractedPeak);
	return newPeakIndex;
}

/**
 * Opens a duplicate peak dialog and saves the new peak to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the peak to duplicate in the database's peak table buffer.
 * @return					The index of the new peak in the database's peak table buffer.
 */
BufferRowIndex openDuplicatePeakDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex)
{
	unique_ptr<Peak> originalPeak = db.getPeakAt(bufferRowIndex);
	originalPeak->peakID = ItemID();
	
	const QString windowTitle = PeakDialog::tr("New peak");
	
	PeakDialog dialog = PeakDialog(parent, mainWindow, db, duplicateItem, windowTitle, std::move(originalPeak));
	if (dialog.exec() != QDialog::Accepted) {
		return BufferRowIndex();
	}
	
	unique_ptr<Peak> extractedPeak = dialog.extractData();
	
	const BufferRowIndex newPeakIndex = db.peaksTable.addRow(parent, *extractedPeak);
	return newPeakIndex;
}

/**
 * Opens an edit peak dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the peak to edit in the database's peak table buffer.
 * @return					True if any changes were made, false otherwise.
 */
bool openEditPeakDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex)
{
	unique_ptr<Peak> originalPeak = db.getPeakAt(bufferRowIndex);
	const ItemID originalPeakID = originalPeak->peakID;
	
	const QString windowTitle = PeakDialog::tr("Edit peak");
	
	PeakDialog dialog = PeakDialog(parent, mainWindow, db, editItem, windowTitle, std::move(originalPeak));
	if (dialog.exec() != QDialog::Accepted || !dialog.changesMade()) {
		return false;
	}
	
	unique_ptr<Peak> extractedPeak = dialog.extractData();
	
	db.peaksTable.updateRow(parent, FORCE_VALID(originalPeakID), *extractedPeak);
	return true;
}

/**
 * Opens a delete peak dialog and deletes the peak from the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the peaks to delete in the database's peak table buffer.
 * @return					True if any items were deleted, false otherwise.
 */
bool openDeletePeaksDialogAndExecute(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices)
{
	Q_UNUSED(mainWindow);
	if (bufferRowIndices.isEmpty()) return false;
	
	QSet<ValidItemID> peakIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		peakIDs += VALID_ITEM_ID(db.peaksTable.primaryKeyColumn.getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db.whatIf_removeRows(db.peaksTable, peakIDs);
	
	if (Settings::confirmDelete.get()) {
		bool plural = peakIDs.size() > 1;
		QString windowTitle = plural ? PeakDialog::tr("Delete peaks") : PeakDialog::tr("Delete peak");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}

	db.removeRows(parent, db.peaksTable, peakIDs);
	return true;
}
