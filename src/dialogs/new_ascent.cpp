#include "new_ascent.h"
#include "src/dialogs/new_peak.h"
#include "src/dialogs/new_trip.h"
#include "src/dialogs/parse_helper.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <iostream>



NewAscentDialog::NewAscentDialog(QWidget* parent): NewOrEditDialog(parent, tr("ascent"))
{
	setupUi(this);
	
	connect(newPeakButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_newPeak);
	connect(dateCheckbox,			&QCheckBox::stateChanged,			this,	&NewAscentDialog::handle_dateSpecifiedChanged);
	connect(timeCheckbox,			&QCheckBox::stateChanged,			this,	&NewAscentDialog::handle_timeSpecifiedChanged);
	connect(difficultySystemCombo,	&QComboBox::currentIndexChanged,	this,	&NewAscentDialog::handle_difficultySystemChanged);
	connect(newTripButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_newTrip);
	connect(addHikerButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_addHiker);
	
	connect(okButton,				&QPushButton::clicked,				this,	&NewAscentDialog::handle_ok);
	connect(cancelButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_cancel);
}



bool NewAscentDialog::changesMade()
{
	if (!titleLabel->text().isEmpty())				return true;
	if (peakCombo->currentIndex() > 0)				return true;
	if (tripCombo->currentIndex() > 0)				return true;
	if (difficultySystemCombo->currentIndex() > 0)	return true;
	if (difficultyGradeCombo->currentIndex() > 0)	return true;
	if (tripCombo->currentIndex() > 0)				return true;
	// TODO hikers
	// TODO photos
	if (!descriptionEditor->document()->isEmpty())	return true;
	return false;
}



void NewAscentDialog::handle_newPeak()
{
	openNewPeakDialogAndStore(this);
	// TODO
}

void NewAscentDialog::handle_dateSpecifiedChanged()
{
	bool enabled = dateCheckbox->isChecked();
	dateWidget->setEnabled(enabled);
}

void NewAscentDialog::handle_timeSpecifiedChanged()
{
	bool enabled = timeCheckbox->isChecked();
	timeWidget->setEnabled(enabled);
}

void NewAscentDialog::handle_difficultySystemChanged()
{
	bool systemSelected = difficultySystemCombo->currentIndex() > 0;
	difficultyGradeCombo->setEnabled(systemSelected);
	if (systemSelected) {
		difficultyGradeCombo->setPlaceholderText(tr("Select grade"));
	} else {
		difficultyGradeCombo->setPlaceholderText(tr("None"));
	}
}

void NewAscentDialog::handle_newTrip()
{
	openNewTripDialogAndStore(this);
	// TODO
}

void NewAscentDialog::handle_addHiker()
{
	openAddHikerDialog(this);
	// TODO
}

void NewAscentDialog::handle_photosPathBrowse()
{
	QString caption = tr("Select folder with photos of ascent");
	QString path = QFileDialog::getExistingDirectory(this, caption);
}



void NewAscentDialog::handle_ok()
{
	accept();
}



Ascent* openNewAscentDialogAndStore(QWidget* parent)
{
	NewAscentDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Ascent* ascent = new Ascent();
		ascent->title = parseLineedit(dialog.titleTextbox);
		// TODO
		return ascent;
	}
	return nullptr;
}

bool openEditAscentDialog(QWidget* parent, Ascent* ascent)
{
	// TODO
	return false;
}
