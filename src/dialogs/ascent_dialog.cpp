#include "ascent_dialog.h"

#include "src/dialogs/add_hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <iostream>



AscentDialog::AscentDialog(QWidget* parent, Database* db, Ascent* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
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
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}

AscentDialog::~AscentDialog()
{
	delete init;
}



void AscentDialog::populateComboBoxes()
{
	// TODO peakCombo
	
	hikeKind->insertItems(1, Ascent::hikeKindNames);
	
	QStringList difficultySystemNames = QStringList();
	for (auto iter = difficultyNames.constBegin(); iter != difficultyNames.constEnd(); iter++) {
		difficultySystemNames.append(iter->first);
	}
	difficultySystemCombo->insertItems(1, difficultySystemNames);
	
	handle_difficultySystemChanged();
	
	// TODO tripCombo
}



void AscentDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	titleLineEdit->setText(init->title);
	// TODO peakCombo
	dateWidget->setDate(init->date);
	peakIndexSpinner->setValue(init->perDayIndex);
	timeWidget->setTime(init->time);
	hikeKindCombo->setCurrentIndex(init->hikeKind);
	traverseCheckbox->setChecked(init->traverse);
	difficultySystemCombo->setCurrentIndex(init->difficultySystem);
	difficultyGradeCombo->setCurrentIndex(init->difficultyGrade);
	// TODO tripCombo
	// TODO hikersListWidget
	// TODO photosListWidget
	descriptionEditor->setDocument(new QTextDocument(init->description));
}


Ascent* AscentDialog::extractData()
{
	QString			title				= parseLineEdit			(titleLineEdit);
	int				peakID				= parseIDCombo			(peakCombo);
	QDate			date				= parseDateWidget		(dateWidget);
	int				perDayIndex			= parseSpinner			(peakIndexSpinner);
	QTime			time				= parseTimeWidget		(timeWidget);
	int				hikeKind			= parseEnumCombo		(hikeKindCombo);
	bool			traverse			= parseCheckbox			(traverseCheckbox);
	int				difficultySystem	= parseEnumCombo		(difficultySystemCombo);
	int				difficultyGrade		= parseEnumCombo		(difficultyGradeCombo);
	int				tripID				= parseIDCombo			(tripCombo);
	QList<int>		hikerIDs			= parseHikerList		(hikersListWidget);
	QList<QString>	photos				= parsePhotosList		(photosListWidget);
	QString			description			= parsePlainTextEdit	(descriptionEditor);
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
	openNewPeakDialogAndStore(this, db);
	// TODO
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
	if (systemSelected) {
		difficultyGradeCombo->setPlaceholderText(tr("Select grade"));
		difficultyGradeCombo->insertItems(1, Ascent::difficultyNames.at(system).second);
	} else {
		difficultyGradeCombo->clear();
		difficultyGradeCombo->setPlaceholderText(tr("None"));
	}
}

void AscentDialog::handle_newTrip()
{
	openNewTripDialogAndStore(this, db);
	// TODO
}

void AscentDialog::handle_addHiker()
{
	openAddHikerDialog(this, db);
	// TODO
}

void AscentDialog::handle_photosPathBrowse()
{
	QString caption = tr("Select folder with photos of ascent");
	QString path = QFileDialog::getExistingDirectory(this, caption);
}



void AscentDialog::handle_ok()
{
	accept();
}



Ascent* openNewAscentDialogAndStore(QWidget* parent, Database* db)
{
	Ascent* newAscent = nullptr;
	
	AscentDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		newAscent = dialog.extractData();
		int ascentID = db->ascentsTable->addRow(newAscent);
		newAscent->ascentID = ascentID;
	}
	
	return newAscent;
}

Ascent* openEditAscentDialog(QWidget* parent, Database* db, Ascent* originalAscent)
{
	Ascent* editedAscent = nullptr;
	
	AscentDialog dialog(parent, db, originalAscent);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		editedAscent = dialog.extractData();
		// TODO update database
	}
	
	delete originalAscent;
	return editedAscent;
}
