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
 * @file ascent_dialog.cpp
 * 
 * This file defines the AscentDialog class.
 */

#include "ascent_dialog.h"

#include "src/dialogs/add_hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/dialogs/parse_helper.h"
#include "src/db/column.h"
#include "src/settings/settings.h"
#include "src/data/enum_names.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <QImageReader>



/**
 * Creates a new ascent dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, initializes hiker and photo list
 * models, connects interactive UI elements, sets initial values, and performs purpose-specific
 * preparations.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 * @param init		The ascent data to initialize the dialog with and store as initial data. AscentDialog takes ownership of this pointer.
 */
AscentDialog::AscentDialog(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* init) :
		ItemDialog(parent, db, purpose),
		init(init),
		selectableRegionIDs(QList<ValidItemID>()),
		selectablePeakIDs(QList<ValidItemID>()),
		selectableTripIDs(QList<ValidItemID>()),
		hikersModel(HikersOnAscent()),
		photosModel(PhotosOfAscent())
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/ascent_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::ascentDialog_geometry);
	
	
	populateComboBoxes();
	
	hikersListView->setModel(&hikersModel);
	hikersListView->setModelColumn(1);
	photosListView->setModel(&photosModel);
	photosListView->setModelColumn(0);
	
	
	connect(regionFilterCombo,					&QComboBox::currentIndexChanged,		this,	&AscentDialog::handle_regionFilterChanged);
	connect(newPeakButton,						&QPushButton::clicked,					this,	&AscentDialog::handle_newPeak);
	connect(dateCheckbox,						&QCheckBox::stateChanged,				this,	&AscentDialog::handle_dateSpecifiedChanged);
	connect(timeCheckbox,						&QCheckBox::stateChanged,				this,	&AscentDialog::handle_timeSpecifiedChanged);
	connect(elevationGainCheckbox,				&QCheckBox::stateChanged,				this,	&AscentDialog::handle_elevationGainSpecifiedChanged);
	connect(difficultySystemCombo,				&QComboBox::currentIndexChanged,		this,	&AscentDialog::handle_difficultySystemChanged);
	connect(newTripButton,						&QPushButton::clicked,					this,	&AscentDialog::handle_newTrip);
	connect(addHikerButton,						&QPushButton::clicked,					this,	&AscentDialog::handle_addHiker);
	connect(removeHikersButton,					&QPushButton::clicked,					this,	&AscentDialog::handle_removeHikers);
	connect(addPhotosButton,					&QPushButton::clicked,					this,	&AscentDialog::handle_addPhotos);
	connect(removePhotosButton,					&QPushButton::clicked,					this,	&AscentDialog::handle_removePhotos);
	connect(photosListView->selectionModel(),	&QItemSelectionModel::selectionChanged,	this,	&AscentDialog::handle_photoSelectionChanged);
	
	connect(okButton,							&QPushButton::clicked,					this,	&AscentDialog::handle_ok);
	connect(cancelButton,						&QPushButton::clicked,					this,	&AscentDialog::handle_cancel);
	
	
	// Set initial date
	dateCheckbox->setChecked(Settings::ascentDialog_dateEnabledInitially.get());
	handle_dateSpecifiedChanged();
	QDate initialDate = QDateTime::currentDateTime().date().addDays(- Settings::ascentDialog_initialDateDaysInPast.get());
	dateWidget->setDate(initialDate);
	// Set initial time
	timeCheckbox->setChecked(Settings::ascentDialog_timeEnabledInitially.get());
	handle_timeSpecifiedChanged();
	timeWidget->setTime(Settings::ascentDialog_initialTime.get());
	// Set initial elevation gain
	elevationGainCheckbox->setChecked(Settings::ascentDialog_elevationGainEnabledInitially.get());
	handle_elevationGainSpecifiedChanged();
	elevationGainSpinner->setValue(Settings::ascentDialog_initialElevationGain.get());
	// Set initial hiker
	ItemID defaultHikerID = db->projectSettings->defaultHiker->get();
	if (defaultHikerID.isValid()) {
		Hiker* hiker = db->getHiker(FORCE_VALID(defaultHikerID));
		hikersModel.addHiker(hiker);
		delete hiker;
	}
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	case duplicateItem:
		Ascent* blankAscent = extractData();
		insertInitData();
		this->init = blankAscent;
		break;
	}
}

/**
 * Destroys the ascent dialog.
 */
AscentDialog::~AscentDialog()
{
	delete init;
}



/**
 * Returns the window title to use when the dialog is used to edit an item.
 * 
 * @return	The window title for editing an item
 */
QString AscentDialog::getEditWindowTitle()
{
	return tr("Edit ascent");
}



/**
 * Populates the dialog's combo boxes with data from the database.
 */
void AscentDialog::populateComboBoxes()
{
	populateItemCombo(db->regionsTable, db->regionsTable->nameColumn, true, regionFilterCombo, selectableRegionIDs, tr("All regions (no filter)"));
	populateItemCombo(db->peaksTable, db->peaksTable->nameColumn, true, peakCombo, selectablePeakIDs);
	
	hikeKindCombo->insertItems(1, EnumNames::translateList(EnumNames::hikeKindNames));
	
	QStringList difficultySystemNames = QStringList();
	std::transform(
			EnumNames::difficultyNames.constBegin(),
			EnumNames::difficultyNames.constEnd(),
			std::back_inserter(difficultySystemNames),
			[](QPair<QString, QStringList> qPair){ return qPair.first; }
	);
	difficultySystemCombo->insertItems(1, EnumNames::translateList(difficultySystemNames));
	
	handle_difficultySystemChanged();
	
	populateItemCombo(db->tripsTable, db->tripsTable->nameColumn, true, tripCombo, selectableTripIDs);
}



/**
 * Inserts the data from the initial ascent object into the dialog's UI elements.
 */
void AscentDialog::insertInitData()
{
	// Title
	titleLineEdit->setText(init->title);
	//  Peak
	if (init->peakID.isValid()) {
		peakCombo->setCurrentIndex(selectablePeakIDs.indexOf(ID_GET(init->peakID)) + 1);	// 0 is None
	} else {
		peakCombo->setCurrentIndex(0);
	}
	// Date
	bool dateSpecified = init->dateSpecified();
	dateCheckbox->setChecked(dateSpecified);
	if (dateSpecified) {
		dateWidget->setDate(init->date);
	}	
	handle_dateSpecifiedChanged();
	// Peak on day
	peakIndexSpinner->setValue(init->perDayIndex);
	// Time
	bool timeSpecified = init->timeSpecified();
	timeCheckbox->setChecked(timeSpecified);
	if (timeSpecified) {
		timeWidget->setTime(init->time);
	}	
	handle_timeSpecifiedChanged();
	// Elevation gain
	bool elevationGainSpecified = init->elevationGainSpecified();
	elevationGainCheckbox->setChecked(elevationGainSpecified);
	if (elevationGainSpecified) {
		elevationGainSpinner->setValue(init->elevationGain);
	}
	handle_elevationGainSpecifiedChanged();
	// Kind of hike
	hikeKindCombo->setCurrentIndex(init->hikeKind);
	// Traverse
	traverseCheckbox->setChecked(init->traverse);
	// Difficulty
	difficultySystemCombo->setCurrentIndex(init->difficultySystem);
	difficultyGradeCombo->setCurrentIndex(init->difficultyGrade);
	// Trip
	if (init->tripID.isValid()) {
		tripCombo->setCurrentIndex(selectableTripIDs.indexOf(ID_GET(init->tripID)) + 1);	// 0 is None
	} else {
		tripCombo->setCurrentIndex(0);
	}
	// Hikers
	hikersModel.clear();
	for (const ValidItemID& hikerID : init->hikerIDs) {
		Hiker* hiker = db->getHiker(hikerID);
		hikersModel.addHiker(hiker);
		delete hiker;
	}
	// Photos
	photosModel.addPhotos(init->photos);
	descriptionEditor->setPlainText(init->description);
}


/**
 * Extracts the data from the UI elements and returns it as an ascent object.
 * 
 * @return	The ascent data as an ascent object. The caller takes ownership of the object.
 */
Ascent* AscentDialog::extractData()
{
	QString				title				= parseLineEdit			(titleLineEdit);
	ItemID				peakID				= parseItemCombo		(peakCombo, selectablePeakIDs);
	QDate				date				= parseDateWidget		(dateWidget);
	int					perDayIndex			= parseSpinner			(peakIndexSpinner);
	QTime				time				= parseTimeWidget		(timeWidget);
	int					elevationGain		= parseSpinner			(elevationGainSpinner);
	int					hikeKind			= parseEnumCombo		(hikeKindCombo, false);
	bool				traverse			= parseCheckbox			(traverseCheckbox);
	int					difficultySystem	= parseEnumCombo		(difficultySystemCombo, true);
	int					difficultyGrade		= parseEnumCombo		(difficultyGradeCombo, true);
	ItemID				tripID				= parseItemCombo		(tripCombo, selectableTripIDs);
	QString				description			= parsePlainTextEdit	(descriptionEditor);
	QSet<ValidItemID>	hikerIDs			= hikersModel.getHikerIDSet();
	QList<Photo>		photos				= photosModel.getPhotoList();
	
	if (!dateCheckbox->isChecked())				date = QDate();
	if (!timeCheckbox->isChecked())				time = QTime();
	if (!elevationGainCheckbox->isChecked())	elevationGain = -1;
	if (difficultySystem < 1 || difficultyGrade < 1) {
		difficultySystem	= 0;
		difficultyGrade		= 0;
	}
	
	Ascent* ascent = new Ascent(ItemID(), title, peakID, date, perDayIndex, time, elevationGain, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, hikerIDs, photos, description);
	return ascent;
}


/**
 * Checks whether changes have been made to the ascent, compared to the initial ascent object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool AscentDialog::changesMade()
{
	Ascent* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



/**
 * Event handler for changes in the region filter combo box.
 * 
 * Updates the peak combo box according to the selected region filter.
 */
void AscentDialog::handle_regionFilterChanged()
{
	ItemID regionID = parseItemCombo(regionFilterCombo, selectableRegionIDs);
	populateItemCombo(db->peaksTable, db->peaksTable->nameColumn, true, peakCombo, selectablePeakIDs, QString(), db->peaksTable->regionIDColumn, regionID);
}

/**
 * Event handler for the new peak button.
 * 
 * Opens a new peak dialog, adds the new peak to the peak combo box and selects it.
 */
void AscentDialog::handle_newPeak()
{
	BufferRowIndex newPeakIndex = openNewPeakDialogAndStore(this, db);
	if (newPeakIndex.isInvalid()) return;
	
	populateItemCombo(db->peaksTable, db->peaksTable->nameColumn, true, peakCombo, selectablePeakIDs);
	ValidItemID peakID = db->peaksTable->getPrimaryKeyAt(newPeakIndex);
	regionFilterCombo->setCurrentIndex(0);
	peakCombo->setCurrentIndex(selectablePeakIDs.indexOf(peakID) + 1);	// 0 is None
}

/**
 * Event handler for changes in the date specified checkbox.
 * 
 * Enables or disables the date widget according to the checkbox's state.
 */
void AscentDialog::handle_dateSpecifiedChanged()
{
	bool enabled = dateCheckbox->isChecked();
	dateWidget->setEnabled(enabled);
}

/**
 * Event handler for changes in the time specified checkbox.
 * 
 * Enables or disables the time widget according to the checkbox's state.
 */
void AscentDialog::handle_timeSpecifiedChanged()
{
	bool enabled = timeCheckbox->isChecked();
	timeWidget->setEnabled(enabled);
}

/**
 * Event handler for changes in the elevation gain specified checkbox.
 * 
 * Enables or disables the elevation gain spinner according to the checkbox's state.
 */
void AscentDialog::handle_elevationGainSpecifiedChanged()
{
	bool enabled = elevationGainCheckbox->isChecked();
	elevationGainSpinner->setEnabled(enabled);
}

/**
 * Event handler for changes in the difficulty system combo box.
 * 
 * Enables or disables and repopulates the difficulty grade combo box according to the selected
 * difficulty system.
 */
void AscentDialog::handle_difficultySystemChanged()
{
	int system = difficultySystemCombo->currentIndex();
	bool systemSelected = system > 0;
	difficultyGradeCombo->setEnabled(systemSelected);
	
	difficultyGradeCombo->clear();
	if (systemSelected) {
		difficultyGradeCombo->setPlaceholderText(tr("Select grade"));
		QStringList translatedList = EnumNames::translateList(EnumNames::difficultyNames.at(system).second);
		difficultyGradeCombo->insertItems(1, translatedList);
	} else {
		difficultyGradeCombo->setPlaceholderText(tr("None"));
	}
}

/**
 * Event handler for the new trip button.
 * 
 * Opens a new trip dialog, adds the new trip to the trip combo box and selects it.
 */
void AscentDialog::handle_newTrip()
{
	BufferRowIndex newTripIndex = openNewTripDialogAndStore(this, db);
	if (newTripIndex.isInvalid()) return;
	
	populateItemCombo(db->tripsTable, db->tripsTable->nameColumn, true, tripCombo, selectableTripIDs);
	ValidItemID tripID = db->tripsTable->getPrimaryKeyAt(newTripIndex);
	tripCombo->setCurrentIndex(selectableTripIDs.indexOf(tripID) + 1);	// 0 is None
}

/**
 * Event handler for the add hiker button.
 * 
 * Opens an add hiker dialog and adds the new hiker to the hikers list.
 */
void AscentDialog::handle_addHiker()
{
	ItemID hikerID = openAddHikerDialog(this, db);
	if (hikerID.isInvalid()) return;
	if (hikersModel.containsHiker(FORCE_VALID(hikerID))) return;
	Hiker* hiker = db->getHiker(FORCE_VALID(hikerID));
	hikersModel.addHiker(hiker);
	delete hiker;
}

/**
 * Event handler for the remove hikers button.
 * 
 * Removes the selected hikers from the hikers list.
 */
void AscentDialog::handle_removeHikers()
{
	QItemSelectionModel* selectionModel = hikersListView->selectionModel();
	if (!selectionModel->hasSelection()) return;
	QModelIndexList selectedIndices = selectionModel->selectedRows();
	for (const QModelIndex& selectedIndex : selectedIndices) {
		hikersModel.removeHikerAt(selectedIndex.row());
	}
}

/**
 * Event handler for the add photos button.
 * 
 * Opens a file dialog for selecting photos and appends the selected photos to the photos list.
 */
void AscentDialog::handle_addPhotos()
{
	// Determine path at which file dialog will open
	int photoIndexForDir = -1;
	QItemSelectionModel* selectionModel = photosListView->selectionModel();
	if (selectionModel->hasSelection()) {	// Use first selected photo if possible
		photoIndexForDir = selectionModel->selectedRows().first().row();
	} else if (!photosModel.isEmpty()) {	// Otherwise, use first photo if it exists
		photoIndexForDir = 0;
	}
	QString preSelectedDir = QString();
	if (photoIndexForDir >= 0) {
		preSelectedDir = QFileInfo(photosModel.getFilepathAt(photoIndexForDir)).path();
	}
	
	QStringList filepaths = openFileDialogForMultiPhotoSelection(this, preSelectedDir);
	if (filepaths.isEmpty()) return;
	
	QList<Photo> photos = QList<Photo>();
	for (const QString& filepath : filepaths) {
		photos.append(Photo(ItemID(), ItemID(), -1, filepath, QString()));
	}
	
	photosModel.addPhotos(photos);
}

/**
 * Event handler for the remove photos button.
 * 
 * Removes the selected photos from the photos list.
 */
void AscentDialog::handle_removePhotos()
{
	QItemSelectionModel* selectionModel = photosListView->selectionModel();
	if (!selectionModel->hasSelection()) return;
	QModelIndexList selectedIndices = selectionModel->selectedRows();
	for (const QModelIndex& selectedIndex : selectedIndices) {
		photosModel.removePhotoAt(selectedIndex.row());
	}
}

/**
 * Event handler for changes in the photo selection.
 * 
 * If exactly one photo was selected before, that photo's description is saved.
 * If exactly one photo is now selected, that photo's description is loaded into the description
 * line edit. If no photo is selected, the description line edit is cleared and disabled. If more
 * than one photo is selected, the description line edit is cleared and disabled.
 * 
 * @param selected		The photos now selected.
 * @param deselected	The photos which were selected before the change.
 */
void AscentDialog::handle_photoSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	// Save description for previously selected photo, if applicable
	savePhotoDescriptionToList(selected, deselected);
	
	QSet<int> nowSelectedRows = QSet<int>();
	for (QModelIndex& index : photosListView->selectionModel()->selectedIndexes()) {
		nowSelectedRows.insert(index.row());
	}

	if (nowSelectedRows.size() == 1) {			// Load description
		QList<int> nowSelectedRowIndexValues = nowSelectedRows.values();
		int rowIndex = nowSelectedRowIndexValues.first();
		QString description = photosModel.getDescriptionAt(rowIndex);
		photoDescriptionLineEdit->setText(description);
		photoDescriptionLineEdit->setEnabled(true);
	} else {									// Clear description
		photoDescriptionLineEdit->setEnabled(false);
		photoDescriptionLineEdit->clear();
	}
}



/**
 * Event handler for the OK button.
 * 
 * Prepares the dialog for closing and then accepts it.
 */
void AscentDialog::handle_ok()
{
	aboutToClose();
	
	accept();
}

/**
 * Prepares the dialog for closing by saving its geometry and photo description.
 */
void AscentDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::ascentDialog_geometry);
	
	savePhotoDescriptionToList();
}

/**
 * Saves the current content of the photo description line edit to the photos list model, iff
 * exactly one photo is selected in the list view.
 * 
 * This method should not be used if the selection has just changed.
 */
void AscentDialog::savePhotoDescriptionToList()
{
	QSet<int> currentlySelectedRowIndices = QSet<int>();
	for (QModelIndex& modelIndex : photosListView->selectionModel()->selectedIndexes()) {
		currentlySelectedRowIndices.insert(modelIndex.row());
	}
	if (currentlySelectedRowIndices.size() == 1) {	// Save description
		int selectedRowIndex = *currentlySelectedRowIndices.begin();
		photosModel.setDescriptionAt(selectedRowIndex, photoDescriptionLineEdit->text());
	}
}

/**
 * After a selection change in the photos list view, saves the current content of the photo
 * description line edit to the photos list model, iff exactly one photo was selected before the
 * change.
 * 
 * This method is intended to be used from an event handler for the selectionChanged signal.
 */
void AscentDialog::savePhotoDescriptionToList(const QItemSelection& selected, const QItemSelection& deselected)
{
	// Find out which rows were selected before the change
	QSet<int> unselectedRows = QSet<int>();
	for (QModelIndex& index : deselected.indexes()) {
		unselectedRows.insert(index.row());
	}
	QSet<int> newlySelectedRows = QSet<int>();
	for (QModelIndex& index : selected.indexes()) {
		newlySelectedRows.insert(index.row());
	}
	QSet<int> nowSelectedRows = QSet<int>();
	for (QModelIndex& index : photosListView->selectionModel()->selectedIndexes()) {
		nowSelectedRows.insert(index.row());
	}
	
	QSet<int> stillSelectedRows = QSet<int>(nowSelectedRows);
	stillSelectedRows.subtract(newlySelectedRows);
	QSet<int> previouslySelectedRows = QSet<int>(unselectedRows);
	previouslySelectedRows.unite(stillSelectedRows);
	
	// If exactly one photo was previously selected, save its description
	if (previouslySelectedRows.size() == 1) {
		int previouslySelectedRowIndex = *previouslySelectedRows.begin();
		photosModel.setDescriptionAt(previouslySelectedRowIndex, photoDescriptionLineEdit->text());
	}
}





static BufferRowIndex openAscentDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* originalAscent);

/**
 * Opens a new ascent dialog and saves the new ascent to the database.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @return			The index of the new ascent in the database's ascent table buffer.
 */
BufferRowIndex openNewAscentDialogAndStore(QWidget* parent, Database* db)
{
	return openAscentDialogAndStore(parent, db, newItem, nullptr);
}

/**
 * Opens a duplicate ascent dialog and saves the new ascent to the database.
 * 
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the ascent to duplicate in the database's ascent table buffer.
 * @return					The index of the new ascent in the database's ascent table buffer.
 */
BufferRowIndex openDuplicateAscentDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Ascent* originalAscent = db->getAscentAt(bufferRowIndex);
	return openAscentDialogAndStore(parent, db, duplicateItem, originalAscent);
}

/**
 * Opens an edit ascent dialog and saves the changes to the database.
 * 
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the ascent to edit in the database's ascent table buffer.
 */
void openEditAscentDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Ascent* originalAscent = db->getAscentAt(bufferRowIndex);
	openAscentDialogAndStore(parent, db, editItem, originalAscent);
}

/**
 * Opens a delete ascent dialog and deletes the ascent from the database.
 * 
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the ascent to delete in the database's ascent table buffer.
 */
void openDeleteAscentDialogAndExecute(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Ascent* ascent = db->getAscentAt(bufferRowIndex);
	ValidItemID ascentID = FORCE_VALID(ascent->ascentID);
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->ascentsTable, ascentID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = AscentDialog::tr("Delete ascent");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	db->removeRow(parent, db->ascentsTable, ascentID);
}



/**
 * Opens a purpose-generic ascent dialog and applies the resulting changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param originalAscent	The ascent data to initialize the dialog with and store as initial data. AscentDialog takes ownership of this pointer.
 * @return					The index of the new ascent in the database's ascent table buffer. Invalid if the dialog was canceled or the purpose was editItem.
 */
static BufferRowIndex openAscentDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* originalAscent)
{
	BufferRowIndex newAscentIndex = BufferRowIndex();
	if (purpose == duplicateItem) {
		assert(originalAscent);
		originalAscent->ascentID = ItemID();
	}
	
	AscentDialog dialog(parent, db, purpose, originalAscent);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Ascent* extractedAscent = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newAscentIndex = db->ascentsTable->addRow(parent, extractedAscent);
			db->participatedTable->addRows(parent, extractedAscent);
			db->photosTable->addRows(parent, extractedAscent);
			break;
		case editItem:
			extractedAscent->ascentID = originalAscent->ascentID;
			
			db->ascentsTable->updateRow(parent, extractedAscent);
			if (originalAscent->hikerIDs != extractedAscent->hikerIDs) {
				db->participatedTable->updateRows(parent, extractedAscent);
			}
			if (originalAscent->photos != extractedAscent->photos) {
				db->photosTable->updateRows(parent, extractedAscent);
			}
			break;
		default:
			assert(false);
		}
		
		delete extractedAscent;
	}
	
	return newAscentIndex;
}



/**
 * Opens a file dialog for selecting a single image file.
 * 
 * @param parent				The parent window.
 * @param preSelectedDir		The directory to open the file dialog in initially, or an empty QString.
 * @param overrideWindowTitle	The window title to use, or an empty QString to use the default one.
 * @return						The selected filepath, or an empty QString if the dialog was cancelled.
 */
QString openFileDialogForSinglePhotoSelection(QWidget* parent, QString preSelectedDir, QString overrideWindowTitle)
{
	QString caption = AscentDialog::tr("Select photo of ascent");
	if (!overrideWindowTitle.isEmpty()) caption = overrideWindowTitle;
	QString filter = getImageFileDialogFilterString();
	QString filepath = QFileDialog::getOpenFileName(parent, caption, preSelectedDir, filter);
	
	QStringList checkedPath = checkFilepathsAndAskUser(parent, {filepath});
	if (checkedPath.isEmpty()) return QString();
	return checkedPath.first();
}

/**
 * Opens a file dialog for selecting multiple image files.
 * 
 * @param parent				The parent window.
 * @param preSelectedDir		The directory to open the file dialog in initially, or an empty QString.
 * @param overrideWindowTitle	The window title to use, or an empty QString to use the default one.
 * @return						The selected filepaths, or an empty QStringList if the dialog was cancelled.
 */
QStringList openFileDialogForMultiPhotoSelection(QWidget* parent, QString preSelectedDir, QString overrideWindowTitle)
{
	QString caption = AscentDialog::tr("Select photos of ascent");
	if (!overrideWindowTitle.isEmpty()) caption = overrideWindowTitle;
	QString filter = getImageFileDialogFilterString();
	QStringList filepaths = QFileDialog::getOpenFileNames(parent, caption, preSelectedDir, filter);
	
	filepaths = checkFilepathsAndAskUser(parent, filepaths);
	
	return filepaths;
}

/**
 * Returns the translated filter string to use for image file dialogs.
 * @return	The translated filter string.
 */
QString getImageFileDialogFilterString()
{
	return AscentDialog::tr("Images")    + " (*.jpg *.jpeg *.png *.bmp *.gif *.pbm *.pgm *.ppm *.xbm *.xpm);;"
		 + AscentDialog::tr("All files") + " (*.*)";
}

/**
 * Checks whether the given filepaths can be read as images and asks the user whether to add them
 * anyway if they cannot.
 * 
 * @param parent		The parent window.
 * @param filepaths		The filepaths to check.
 * @return				The filepaths which are unproblematic  or confirmed by the user. Empty if the user cancels.
 */
QStringList checkFilepathsAndAskUser(QWidget* parent, QStringList filepaths)
{
	bool noToAll = false;
	
	for (int i = 0; i < filepaths.size(); i++) {
		QString filepath = filepaths.at(i);
		QImageReader reader = QImageReader(filepath);
		if (reader.canRead()) continue;
		
		if (noToAll) {
			filepaths.remove(i--);	// Remove this path and skip the dialog
			continue;
		}
		
		QString title = AscentDialog::tr("File error");
		QString message = AscentDialog::tr("Image file cannot be read:\n%1"
										   "\nReason: %2."
										   "\n\nDo you want to use it anyway?")
				.arg(filepath, reader.errorString());
		auto buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		if (filepaths.size() - 1 > i) {
			buttons |= QMessageBox::YesToAll | QMessageBox::NoToAll;
		}
		auto pressedButton = QMessageBox::warning(parent, title, message, buttons);
		
		if (pressedButton == QMessageBox::Yes) {
			// Do nothing
		}
		else if (pressedButton == QMessageBox::YesToAll) {
			break;	// Leave loop, no more paths need to be checked or removed
		}
		else if (pressedButton == QMessageBox::No) {
			filepaths.remove(i--);	// Remove this path
		}
		else if (pressedButton == QMessageBox::NoToAll) {
			filepaths.remove(i--);	// Remove this path and set flag
			noToAll = true;
		}
		else if (pressedButton == QMessageBox::Cancel) {
			return QStringList();	// Return with empty list
		}
		else assert(false);
	}
	
	return filepaths;
}
