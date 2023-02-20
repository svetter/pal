#include "ascent_dialog.h"

#include "src/dialogs/add_hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/dialogs/parse_helper.h"
#include "src/db/column.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <QImageReader>



AscentDialog::AscentDialog(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init),
		hikersModel(HikersOnAscent()),
		photosModel(PhotosOfAscent())
{
	setupUi(this);
	
	populateComboBoxes();
	
	
	connect(newPeakButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_newPeak);
	connect(dateCheckbox,			&QCheckBox::stateChanged,			this,	&AscentDialog::handle_dateSpecifiedChanged);
	connect(timeCheckbox,			&QCheckBox::stateChanged,			this,	&AscentDialog::handle_timeSpecifiedChanged);
	connect(elevationGainCheckbox,	&QCheckBox::stateChanged,			this,	&AscentDialog::handle_elevationGainSpecifiedChanged);
	connect(difficultySystemCombo,	&QComboBox::currentIndexChanged,	this,	&AscentDialog::handle_difficultySystemChanged);
	connect(newTripButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_newTrip);
	connect(addHikerButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_addHiker);
	connect(removeHikersButton,		&QPushButton::clicked,				this,	&AscentDialog::handle_removeHikers);
	connect(addPhotosButton,		&QPushButton::clicked,				this,	&AscentDialog::handle_addPhotos);
	connect(removePhotosButton,		&QPushButton::clicked,				this,	&AscentDialog::handle_removePhotos);
	
	connect(okButton,				&QPushButton::clicked,				this,	&AscentDialog::handle_ok);
	connect(cancelButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_cancel);
	
	
	hikersListView->setModel(&hikersModel);
	hikersListView->setModelColumn(1);	
	photosListView->setModel(&photosModel);
	
	
	QDate initialDate = QDateTime::currentDateTime().date();
	dateWidget->setDate(initialDate);
	
	
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
	peakCombo->setModel(db->peaksTable);
	peakCombo->setRootModelIndex(db->peaksTable->getNullableRootModelIndex());
	peakCombo->setModelColumn(db->peaksTable->nameColumn->getIndex());
	
	hikeKindCombo->insertItems(1, Ascent::hikeKindNames);
	
	QStringList difficultySystemNames = QStringList();
	std::transform(
			Ascent::difficultyNames.constBegin(),
			Ascent::difficultyNames.constEnd(),
			std::back_inserter(difficultySystemNames),
			[](QPair<QString, QStringList> qPair){ return qPair.first; }
	);
	difficultySystemCombo->insertItems(1, difficultySystemNames);
	
	handle_difficultySystemChanged();
	
	tripCombo->setModel(db->tripsTable);
	tripCombo->setRootModelIndex(db->tripsTable->getNullableRootModelIndex());
	tripCombo->setModelColumn(db->tripsTable->nameColumn->getIndex());
}



void AscentDialog::insertInitData()
{
	// Title
	titleLineEdit->setText(init->title);
	//  Peak
	if (init->peakID.isValid()) {
		peakCombo->setCurrentIndex(db->peaksTable->getBufferIndexForPrimaryKey(init->peakID.get()) + 1);	// 0 is None
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
		tripCombo->setCurrentIndex(db->tripsTable->getBufferIndexForPrimaryKey(init->tripID.get()) + 1);	// 0 is None
	} else {
		tripCombo->setCurrentIndex(0);
	}
	// Hikers
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
	ItemID				peakID				= parseIDCombo			(peakCombo);
	QDate				date				= parseDateWidget		(dateWidget);
	int					perDayIndex			= parseSpinner			(peakIndexSpinner);
	QTime				time				= parseTimeWidget		(timeWidget);
	int					elevationGain		= parseSpinner			(elevationGainSpinner);
	int					hikeKind			= parseEnumCombo		(hikeKindCombo, false);
	bool				traverse			= parseCheckbox			(traverseCheckbox);
	int					difficultySystem	= parseEnumCombo		(difficultySystemCombo, true);
	int					difficultyGrade		= parseEnumCombo		(difficultyGradeCombo, true);
	ItemID				tripID				= parseIDCombo			(tripCombo);
	QString				description			= parsePlainTextEdit	(descriptionEditor);
	QSet<ValidItemID>	hikerIDs			= hikersModel.getHikerIDSet();
	QStringList			photos				= photosModel.getPhotoList();
	
	if (!dateCheckbox->isChecked())	date = QDate();	
	if (!timeCheckbox->isChecked())	time = QTime();
	if (difficultySystem < 1 || difficultyGrade < 1) {
		difficultySystem	= -1;
		difficultyGrade		= -1;
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
	if (newPeakIndex >= 0) {
		peakCombo->setCurrentIndex(newPeakIndex);
	}
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
		difficultyGradeCombo->insertItems(1, Ascent::difficultyNames.at(system).second);
	} else {
		difficultyGradeCombo->setPlaceholderText(tr("None"));
	}
}

void AscentDialog::handle_newTrip()
{
	int newTripIndex = openNewTripDialogAndStore(this, db);
	if (newTripIndex < 0) return;
	peakCombo->setCurrentIndex(newTripIndex);
}

void AscentDialog::handle_addHiker()
{
	int hikerID = openAddHikerDialog(this, db);
	if (hikerID < 0) return;
	if (hikersModel.containsHiker(hikerID)) return;
	Hiker* hiker = db->getHiker(hikerID);
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
	QString caption = tr("Select photos of ascent");
	QString preSelectedDir = QString();
	QString filter = tr("Images") + " (*.jpg *.jpeg *.png *.bmp *.gif *.pbm *.pgm *.ppm *.xbm *.xpm);;"
			+ tr("All files") + " (*.*)";
	QStringList filepaths = QFileDialog::getOpenFileNames(this, caption, preSelectedDir, filter, &filter);
	if (filepaths.isEmpty()) return;
	photosModel.addPhotos(filepaths);
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



void AscentDialog::handle_ok()
{
	accept();
}





static int openAscentDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* originalAscent);

int openNewAscentDialogAndStore(QWidget* parent, Database* db)
{
	return openAscentDialogAndStore(parent, db, newItem, nullptr);
}

int openDuplicateAscentDialogAndStore(QWidget* parent, Database* db, Ascent* copyFrom)
{
	return openAscentDialogAndStore(parent, db, duplicateItem, copyFrom);
}

void openEditAscentDialogAndStore(QWidget* parent, Database* db, Ascent* originalAscent)
{
	openAscentDialogAndStore(parent, db, editItem, originalAscent);
}

void openDeleteAscentDialogAndExecute(QWidget* parent, Database* db, Ascent* ascent)
{
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->ascentsTable, ascent->ascentID.forceValid());
	
	QString windowTitle = AscentDialog::tr("Delete ascent");
	bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
	if (!proceed) return;
	
	db->removeRow(parent, db->ascentsTable, ascent->ascentID.forceValid());
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
			db->ascentsTable->updateRow(parent, originalAscent->ascentID.forceValid(), extractedAscent);
			db->participatedTable->updateRows(parent, extractedAscent);
			db->photosTable->updateRows(parent, extractedAscent);
			break;
		default:
			assert(false);
		}
		
		delete extractedAscent;
	}
	
	return newAscentIndex;
}
