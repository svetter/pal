#include "ascent_dialog.h"

#include "src/dialogs/add_hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>



AscentDialog::AscentDialog(QWidget* parent, Database* db, Ascent* init) :
		NewOrEditDialog(parent, db, init != nullptr, tr("Edit ascent")),
		init(init),
		hikersModel(HikersOnAscent()),
		photosModel(PhotosOfAscent())
{
	setupUi(this);
	
	populateComboBoxes();
	
	
	connect(newPeakButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_newPeak);
	connect(dateCheckbox,			&QCheckBox::stateChanged,			this,	&AscentDialog::handle_dateSpecifiedChanged);
	connect(timeCheckbox,			&QCheckBox::stateChanged,			this,	&AscentDialog::handle_timeSpecifiedChanged);
	connect(difficultySystemCombo,	&QComboBox::currentIndexChanged,	this,	&AscentDialog::handle_difficultySystemChanged);
	connect(newTripButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_newTrip);
	connect(addHikerButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_addHiker);
	
	connect(okButton,				&QPushButton::clicked,				this,	&AscentDialog::handle_ok);
	connect(cancelButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_cancel);
	
	
	QDate initialDate = QDateTime::currentDateTime().date();
	dateWidget->setDate(initialDate);
	
	
	if (edit) {	
		changeStringsForEdit(okButton);
		insertInitData();
	} else {
		this->init = extractData();
	}
}

AscentDialog::~AscentDialog()
{
	delete init;
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
	titleLineEdit->setText(init->title);
	peakCombo->setCurrentIndex(db->peaksTable->getBufferIndexForPrimaryKey(init->peakID));
	dateWidget->setDate(init->date);
	peakIndexSpinner->setValue(init->perDayIndex);
	timeWidget->setTime(init->time);
	hikeKindCombo->setCurrentIndex(init->hikeKind);
	traverseCheckbox->setChecked(init->traverse);
	difficultySystemCombo->setCurrentIndex(init->difficultySystem);
	difficultyGradeCombo->setCurrentIndex(init->difficultyGrade);
	tripCombo->setCurrentIndex(db->tripsTable->getBufferIndexForPrimaryKey(init->tripID));
	for (auto iter = init->hikerIDs.constBegin(); iter != init->hikerIDs.constEnd(); iter++) {
		Hiker* hiker = db->getHiker(*iter);
		hikersModel.addHiker(hiker);
		delete hiker;
	}
	for (auto iter = init->hikerIDs.constBegin(); iter != init->hikerIDs.constEnd(); iter++) {
		photosModel.addPhotos(init->photos);
	}
	descriptionEditor->setPlainText(init->description);
}


Ascent* AscentDialog::extractData()
{
	QString		title				= parseLineEdit			(titleLineEdit);
	int			peakID				= parseIDCombo			(peakCombo);
	QDate		date				= parseDateWidget		(dateWidget);
	int			perDayIndex			= parseSpinner			(peakIndexSpinner);
	QTime		time				= parseTimeWidget		(timeWidget);
	int			hikeKind			= parseEnumCombo		(hikeKindCombo, false);
	bool		traverse			= parseCheckbox			(traverseCheckbox);
	int			difficultySystem	= parseEnumCombo		(difficultySystemCombo, true);
	int			difficultyGrade		= parseEnumCombo		(difficultyGradeCombo, true);
	int			tripID				= parseIDCombo			(tripCombo);
	QList<int>	hikerIDs			= parseHikersList		(hikersListView);
	QStringList	photos				= parsePhotosList		(photosListView);
	QString		description			= parsePlainTextEdit	(descriptionEditor);
	if (difficultySystem < 1 || difficultyGrade < 1) {
		difficultySystem	= -1;
		difficultyGrade		= -1;
	}
	Ascent* ascent = new Ascent(-1, title, peakID, date, perDayIndex, time, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, hikerIDs, photos, description);
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
	Hiker* hiker = db->getHiker(hikerID);
	hikersModel.addHiker(hiker);
	delete hiker;
}

void AscentDialog::handle_removeHikers()
{
	// TODO #98 get list of selected rows and remove them
}

void AscentDialog::handle_addPhotos()
{
	QString caption = tr("Select photos of ascent");
	QString preSelectedDir = QString();
	QString filter = tr("Images") + " (*.jpg, *.jpeg, *.png, *.bmp);;" + tr("All") + " (*.*)";
	QStringList filepaths = QFileDialog::getOpenFileNames(this, caption, preSelectedDir, filter, &filter);
	photosModel.addPhotos(filepaths);
}

void AscentDialog::handle_removePhotos()
{
	// TODO #98 get list of selected rows and remove them
}



void AscentDialog::handle_ok()
{
	accept();
}



int openNewAscentDialogAndStore(QWidget* parent, Database* db)
{
	int newAscentIndex = -1;
	
	AscentDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		Ascent* newAscent = dialog.extractData();
		newAscentIndex = db->ascentsTable->addRow(parent, newAscent);
		delete newAscent;
	}
	
	return newAscentIndex;
}

void openEditAscentDialogAndStore(QWidget* parent, Database* db, Ascent* originalAscent)
{
	AscentDialog dialog(parent, db, originalAscent);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		Ascent* editedAscent = dialog.extractData();
		// TODO update database
		delete editedAscent;
	}
}
