#include "new_ascent.h"
#include "src/dialogs/new_peak.h"
#include "src/dialogs/new_trip.h"
#include "src/dialogs/parse_helper.h"

#include <QFileDialog>
#include <QMessageBox>
#include <iostream>



NewAscentDialog::NewAscentDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	
	connect(newPeakButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_newPeak);
	connect(dateCheckbox,			&QCheckBox::stateChanged,			this,	&NewAscentDialog::handle_dateSpecifiedChanged);
	connect(timeCheckbox,			&QCheckBox::stateChanged,			this,	&NewAscentDialog::handle_timeSpecifiedChanged);
	connect(difficultySystemCombo,	&QComboBox::currentIndexChanged,	this,	&NewAscentDialog::handle_difficultySystemChanged);
	connect(newTripButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_newTrip);
	
	connect(okButton,				&QPushButton::clicked,				this,	&NewAscentDialog::handle_ok);
	connect(cancelButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_cancel);
}



bool NewAscentDialog::anyChanges()
{
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
	NewPeakDialog dialog(this);
	dialog.exec();
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
	NewTripDialog dialog(this);
	dialog.exec();
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

void NewAscentDialog::handle_cancel()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new ascent");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewAscentDialog::reject()
{
	handle_cancel();
}



Ascent* openNewAscentDialogAndStore(QWidget *parent)
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

bool openEditAscentDialog(QWidget *parent, Ascent* ascent)
{
	// TODO
	return false;
}
