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
 * @file ascent_dialog.h
 * 
 * This file declares the AscentDialog class.
 */

#include "ascent_dialog.h"

#include "src/dialogs/add_hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/dialogs/parse_helper.h"
#include "src/db/column.h"
#include "src/main/settings.h"
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
		Hiker* hiker = db->getHiker(defaultHikerID.forceValid());
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
		peakCombo->setCurrentIndex(selectablePeakIDs.indexOf(init->peakID.get()) + 1);	// 0 is None
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
		tripCombo->setCurrentIndex(selectableTripIDs.indexOf(init->tripID.get()) + 1);	// 0 is None
	} else {
		tripCombo->setCurrentIndex(0);
	}
	// Hikers
	hikersModel.clear();
	for (auto iter = init->hikerIDs.constBegin(); iter != init->hikerIDs.constEnd(); iter++) {
		Hiker* hiker = db->getHiker(*iter);
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
	if (hikersModel.containsHiker(hikerID.forceValid())) return;
	Hiker* hiker = db->getHiker(hikerID.forceValid());
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
	QModelIndexList selected = selectionModel->selectedRows();
	for (auto iter = selected.constBegin(); iter != selected.constEnd(); iter++) {
		hikersModel.removeHikerAt((*iter).row());
	}
}

/**
 * Event handler for the add photos button.
 * 
 * Opens a file dialog for selecting photos and appends the selected photos to the photos list.
 */
void AscentDialog::handle_addPhotos()
{
	QStringList filepaths = openFileDialogForPhotosSelection(this);
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
	QModelIndexList selected = selectionModel->selectedRows();
	for (auto iter = selected.constBegin(); iter != selected.constEnd(); iter++) {
		photosModel.removePhotoAt((*iter).row());
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
	QSet<int> previouslySelectedRows = getPreviouslySelectedRows(selected, deselected);
	QSet<int> nowSelectedRows = QSet<int>();
	for (QModelIndex& index : photosListView->selectionModel()->selectedIndexes()) {
		nowSelectedRows.insert(index.row());
	}
	
	if (previouslySelectedRows.size() == 1) {	// Save description
		QList<int> previouslySelectedRowIndexValues = previouslySelectedRows.values();
		int rowIndex = previouslySelectedRowIndexValues.first();
		photosModel.setDescriptionAt(rowIndex, photoDescriptionLineEdit->text());
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
	
	handle_photoSelectionChanged();
}



/**
 * From the sets of newly selected and deselected rows in a QItemSelection, returns the set of rows
 * which were selected before the change.
 * 
 * @param selected		The indices of the rows which were newly selected.
 * @param deselected	The indices of the rows which were deselected.
 * @return				The rows which were selected before the change.
 */
QSet<int> AscentDialog::getPreviouslySelectedRows(const QItemSelection& selected, const QItemSelection& deselected) const
{
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
	
	return previouslySelectedRows;
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
	ValidItemID ascentID = ascent->ascentID.forceValid();
	
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
 * Opens a file dialog for selecting photos.
 * 
 * @param parent	The parent window.
 * @return			The selected filepaths.
 */
QStringList openFileDialogForPhotosSelection(QWidget* parent)
{
	QString caption = AscentDialog::tr("Select photos of ascent");
	QString preSelectedDir = QString();
	QString filter = AscentDialog::tr("Images") + " (*.jpg *.jpeg *.png *.bmp *.gif *.pbm *.pgm *.ppm *.xbm *.xpm);;"
			+ AscentDialog::tr("All files") + " (*.*)";
	QStringList filepaths = QFileDialog::getOpenFileNames(parent, caption, preSelectedDir, filter);
	return filepaths;
}
