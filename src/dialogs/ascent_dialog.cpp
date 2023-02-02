#include "ascent_dialog.h"

#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <iostream>



AscentDialog::AscentDialog(QWidget* parent): NewOrEditDialog(parent, tr("ascent"))
{
	setupUi(this);
	
	connect(newPeakButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_newPeak);
	connect(dateCheckbox,			&QCheckBox::stateChanged,			this,	&AscentDialog::handle_dateSpecifiedChanged);
	connect(timeCheckbox,			&QCheckBox::stateChanged,			this,	&AscentDialog::handle_timeSpecifiedChanged);
	connect(difficultySystemCombo,	&QComboBox::currentIndexChanged,	this,	&AscentDialog::handle_difficultySystemChanged);
	connect(newTripButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_newTrip);
	connect(addHikerButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_addHiker);
	
	connect(okButton,				&QPushButton::clicked,				this,	&AscentDialog::handle_ok);
	connect(cancelButton,			&QPushButton::clicked,				this,	&AscentDialog::handle_cancel);
}



bool AscentDialog::changesMade()
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



void AscentDialog::handle_newPeak()
{
	openNewPeakDialogAndStore(this);
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
	bool systemSelected = difficultySystemCombo->currentIndex() > 0;
	difficultyGradeCombo->setEnabled(systemSelected);
	if (systemSelected) {
		difficultyGradeCombo->setPlaceholderText(tr("Select grade"));
	} else {
		difficultyGradeCombo->setPlaceholderText(tr("None"));
	}
}

void AscentDialog::handle_newTrip()
{
	openNewTripDialogAndStore(this);
	// TODO
}

void AscentDialog::handle_addHiker()
{
	openAddHikerDialog(this);
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



Ascent* openNewAscentDialogAndStore(QWidget* parent)
{
	AscentDialog dialog(parent);
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
