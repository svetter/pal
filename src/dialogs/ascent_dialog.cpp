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



AscentDialog::AscentDialog(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init),
		selectablePeakIDs(QList<ValidItemID>()),
		selectableTripIDs(QList<ValidItemID>()),
		hikersModel(HikersOnAscent()),
		photosModel(PhotosOfAscent())
{
	setupUi(this);
	
	restoreDialogGeometry(this, parent, &Settings::ascentDialog_geometry);
	
	
	populateComboBoxes();
	
	hikersListView->setModel(&hikersModel);
	hikersListView->setModelColumn(1);
	photosListView->setModel(&photosModel);
	photosListView->setModelColumn(0);
	
	
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

AscentDialog::~AscentDialog()
{
	delete init;
}



QString AscentDialog::getEditWindowTitle()
{
	return tr("Edit ascent");
}



void AscentDialog::populateComboBoxes()
{
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


bool AscentDialog::changesMade()
{
	Ascent* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void AscentDialog::handle_newPeak()
{
	int newPeakIndex = openNewPeakDialogAndStore(this, db);
	if (newPeakIndex < 0) return;
	
	populateItemCombo(db->peaksTable, db->peaksTable->nameColumn, true, peakCombo, selectablePeakIDs);
	ValidItemID peakID = db->peaksTable->getPrimaryKeyAt(newPeakIndex);
	peakCombo->setCurrentIndex(selectablePeakIDs.indexOf(peakID) + 1);	// 0 is None
}

void AscentDialog::handle_dateSpecifiedChanged()
{
	bool enabled = dateCheckbox->isChecked();
	dateWidget->setEnabled(enabled);
}

void AscentDialog::handle_timeSpecifiedChanged()
{
	bool enabled = timeCheckbox->isChecked();
	timeWidget->setEnabled(enabled);
}

void AscentDialog::handle_elevationGainSpecifiedChanged()
{
	bool enabled = elevationGainCheckbox->isChecked();
	elevationGainSpinner->setEnabled(enabled);
}

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

void AscentDialog::handle_newTrip()
{
	int newTripIndex = openNewTripDialogAndStore(this, db);
	if (newTripIndex < 0) return;
	
	populateItemCombo(db->tripsTable, db->tripsTable->nameColumn, true, tripCombo, selectableTripIDs);
	ValidItemID tripID = db->rangesTable->getPrimaryKeyAt(newTripIndex);
	tripCombo->setCurrentIndex(selectableTripIDs.indexOf(tripID) + 1);	// 0 is None
}

void AscentDialog::handle_addHiker()
{
	ItemID hikerID = openAddHikerDialog(this, db);
	if (hikerID.isInvalid()) return;
	if (hikersModel.containsHiker(hikerID.forceValid())) return;
	Hiker* hiker = db->getHiker(hikerID.forceValid());
	hikersModel.addHiker(hiker);
	delete hiker;
}

void AscentDialog::handle_removeHikers()
{
	QItemSelectionModel* selectionModel = hikersListView->selectionModel();
	if (!selectionModel->hasSelection()) return;
	QModelIndexList selected = selectionModel->selectedRows();
	for (auto iter = selected.constBegin(); iter != selected.constEnd(); iter++) {
		hikersModel.removeHikerAt((*iter).row());
	}
}

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

void AscentDialog::handle_removePhotos()
{
	QItemSelectionModel* selectionModel = photosListView->selectionModel();
	if (!selectionModel->hasSelection()) return;
	QModelIndexList selected = selectionModel->selectedRows();
	for (auto iter = selected.constBegin(); iter != selected.constEnd(); iter++) {
		photosModel.removePhotoAt((*iter).row());
	}
}

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



void AscentDialog::handle_ok()
{
	aboutToClose();
	
	accept();
}

void AscentDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::ascentDialog_geometry);
	
	handle_photoSelectionChanged();
}



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





static int openAscentDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* originalAscent);

int openNewAscentDialogAndStore(QWidget* parent, Database* db)
{
	return openAscentDialogAndStore(parent, db, newItem, nullptr);
}

int openDuplicateAscentDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Ascent* originalAscent = db->getAscentAt(bufferRowIndex);
	return openAscentDialogAndStore(parent, db, duplicateItem, originalAscent);
}

void openEditAscentDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Ascent* originalAscent = db->getAscentAt(bufferRowIndex);
	openAscentDialogAndStore(parent, db, editItem, originalAscent);
}

void openDeleteAscentDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex)
{
	Ascent* ascent = db->getAscentAt(bufferRowIndex);
	ValidItemID ascentID = ascent->ascentID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->ascentsTable, ascentID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = AscentDialog::tr("Delete ascent");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	// Remove ascent itself
	db->removeRow(parent, db->ascentsTable, ascentID);
}



static int openAscentDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* originalAscent)
{
	int newAscentIndex = -1;
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



QStringList openFileDialogForPhotosSelection(QWidget* parent)
{
	QString caption = AscentDialog::tr("Select photos of ascent");
	QString preSelectedDir = QString();
	QString filter = AscentDialog::tr("Images") + " (*.jpg *.jpeg *.png *.bmp *.gif *.pbm *.pgm *.ppm *.xbm *.xpm);;"
			+ AscentDialog::tr("All files") + " (*.*)";
	QStringList filepaths = QFileDialog::getOpenFileNames(parent, caption, preSelectedDir, filter);
	return filepaths;
}
