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
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param init				The peak data to initialize the dialog with and store as initial data. PeakDialog takes ownership of this pointer.
 * @param numItemsToEdit	The number of items to edit, if the purpose is multi-edit.
 */
PeakDialog::PeakDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Peak> init, int numItemsToEdit) :
	ItemDialog(parent, mainWindow, db, purpose),
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
	
	setWindowIcon(QIcon(":/icons/ico/logo_peak_multisize_square.ico"));
	switch (purpose) {
	case newItem:
	case duplicateItem:	setWindowTitle(tr("New peak"));								break;
	case editItem:
	case multiEdit:		setWindowTitle(tr("Edit %Ln peak(s)", "", numItemsToEdit));	break;
	}
	
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
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
	case multiEdit:
		insertInitData();
		break;
	case duplicateItem:
		unique_ptr<Peak> blankPeak = extractData();
		insertInitData();
		this->init = std::move(blankPeak);
		break;
	}
	changeUIForPurpose();
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
	auto callWhenDone = [this](const BufferRowIndex newRegionIndex) {
		if (newRegionIndex.isInvalid()) return;
		
		populateRegionCombo(db, *regionCombo, selectableRegionIDs);
		const ValidItemID newRegionID = db.regionsTable.getPrimaryKeyAt(newRegionIndex);
		regionCombo->setCurrentIndex(selectableRegionIDs.indexOf(newRegionID) + 1);	// 0 is None
	};
	
	openNewRegionDialogAndStore(*this, mainWindow, db, callWhenDone);
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
 * @param callWhenDone	The function to call after the dialog has closed.
 */
void openNewPeakDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, std::function<void (BufferRowIndex)> callWhenDone)
{
	PeakDialog* dialog = new PeakDialog(parent, mainWindow, db, newItem, nullptr);
	
	auto callWhenClosed = [=, &parent, &db]() {
		BufferRowIndex newPeakIndex = BufferRowIndex();
		
		if (dialog->result() == QDialog::Accepted) {
			unique_ptr<Peak> extractedPeak = dialog->extractData();
			
			db.beginChangingData();
			newPeakIndex = db.peaksTable.addRow(parent, *extractedPeak);
			db.finishChangingData();
		}
		
		delete dialog;
		return callWhenDone(newPeakIndex);
	};
	PeakDialog::connect(dialog, &PeakDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a duplicate peak dialog and saves the new peak to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the peak to duplicate in the database's peak table buffer.
 * @param callWhenDone		The function to call after the dialog has closed.
 */
void openDuplicatePeakDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (BufferRowIndex)> callWhenDone)
{
	unique_ptr<Peak> originalPeak = db.getPeakAt(bufferRowIndex);
	originalPeak->peakID = ItemID();
	
	PeakDialog* dialog = new PeakDialog(parent, mainWindow, db, duplicateItem, std::move(originalPeak));
	
	auto callWhenClosed = [=, &parent, &db]() {
		BufferRowIndex newPeakIndex = BufferRowIndex();
		
		if (dialog->result() == QDialog::Accepted) {
			unique_ptr<Peak> extractedPeak = dialog->extractData();
			
			db.beginChangingData();
			newPeakIndex = db.peaksTable.addRow(parent, *extractedPeak);
			db.finishChangingData();
		}
		
		delete dialog;
		return callWhenDone(newPeakIndex);
	};
	PeakDialog::connect(dialog, &PeakDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens an edit peak dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the peak to edit in the database's peak table buffer.
 * @param callWhenDone		The function to call after the dialog has closed.
 */
void openEditPeakDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (bool)> callWhenDone)
{
	unique_ptr<Peak> originalPeak = db.getPeakAt(bufferRowIndex);
	const ItemID originalPeakID = originalPeak->peakID;
	
	PeakDialog* dialog = new PeakDialog(parent, mainWindow, db, editItem, std::move(originalPeak));
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Peak> extractedPeak = dialog->extractData();
			
			db.beginChangingData();
			db.peaksTable.updateRow(parent, FORCE_VALID(originalPeakID), *extractedPeak);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	PeakDialog::connect(dialog, &PeakDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a multi-edit peak dialog and saves the changes to the database.
 * 
 * @param parent				The parent window.
 * @param mainWindow			The application's main window.
 * @param db					The project database.
 * @param bufferRowIndices		The buffer row indices of the peaks to edit.
 * @param initBufferRowIndex	The index of the peak whose data to initialize the dialog with.
 * @param callWhenDone			The function to call after the dialog has closed.
 */
void openMultiEditPeaksDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices, BufferRowIndex initBufferRowIndex, std::function<void (bool)> callWhenDone)
{
	assert(!bufferRowIndices.isEmpty());
	
	unique_ptr<Peak> originalPeak = db.getPeakAt(initBufferRowIndex);
	
	PeakDialog* dialog = new PeakDialog(parent, mainWindow, db, multiEdit, std::move(originalPeak), bufferRowIndices.size());
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Peak> extractedPeak = dialog->extractData();
			extractedPeak->peakID = ItemID();
			QSet<const Column*> columnsToSave = dialog->getMultiEditColumns();
			QList<const Column*> columnList = QList<const Column*>(columnsToSave.constBegin(), columnsToSave.constEnd());
			
			db.beginChangingData();
			db.peaksTable.updateRows(parent, bufferRowIndices, columnList, *extractedPeak);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	PeakDialog::connect(dialog, &PeakDialog::finished, callWhenClosed);
	
	dialog->open();
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
		const QString windowTitle = PeakDialog::tr("Delete %Ln peak(s)", "", peakIDs.size());
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}
	
	db.beginChangingData();
	db.removeRows(parent, db.peaksTable, peakIDs);
	db.finishChangingData();
	return true;
}
