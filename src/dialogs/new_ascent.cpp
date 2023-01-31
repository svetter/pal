#include "new_ascent.h"
#include "src/dialogs/new_peak.h"
#include "src/dialogs/new_trip.h"

#include <QFileDialog>
#include <QMessageBox>
#include <iostream>



NewAscentDialog::NewAscentDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewAscentDialog)
{
	ui->setupUi(this);
	
	connect(ui->newPeakButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_newPeak);
	connect(ui->dateCheckbox,			&QCheckBox::stateChanged,			this,	&NewAscentDialog::handle_dateSpecifiedChanged);
	connect(ui->timeCheckbox,			&QCheckBox::stateChanged,			this,	&NewAscentDialog::handle_timeSpecifiedChanged);
	connect(ui->difficultySystemCombo,	&QComboBox::currentIndexChanged,	this,	&NewAscentDialog::handle_difficultySystemChanged);
	connect(ui->newTripButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_newTrip);
	connect(ui->cancelButton,			&QPushButton::clicked,				this,	&NewAscentDialog::handle_close);
}

NewAscentDialog::~NewAscentDialog()
{
	delete ui;
}



bool NewAscentDialog::anyChanges()
{
	if (ui->peakCombo->currentIndex() > 0)				return true;
	if (ui->tripCombo->currentIndex() > 0)				return true;
	if (ui->difficultySystemCombo->currentIndex() > 0)	return true;
	if (ui->difficultyGradeCombo->currentIndex() > 0)	return true;
	if (ui->tripCombo->currentIndex() > 0)				return true;
	// TODO hikers
	// TODO photos
	if (!ui->descriptionEditor->document()->isEmpty())	return true;
	return false;
}



void NewAscentDialog::handle_newPeak()
{
	NewPeakDialog dialog(this);
	dialog.exec();
}

void NewAscentDialog::handle_dateSpecifiedChanged()
{
	bool enabled = ui->dateCheckbox->isChecked();
	ui->dateWidget->setEnabled(enabled);
}

void NewAscentDialog::handle_timeSpecifiedChanged()
{
	bool enabled = ui->timeCheckbox->isChecked();
	ui->timeWidget->setEnabled(enabled);
}

void NewAscentDialog::handle_difficultySystemChanged()
{
	bool systemSelected = ui->difficultySystemCombo->currentIndex() > 0;
	ui->difficultyGradeCombo->setEnabled(systemSelected);
	if (systemSelected) {
		ui->difficultyGradeCombo->setPlaceholderText(tr("Select grade"));
	} else {
		ui->difficultyGradeCombo->setPlaceholderText(tr("None"));
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

void NewAscentDialog::handle_close()
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
	handle_close();
}
