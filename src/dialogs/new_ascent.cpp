#include "new_ascent.h"
#include "src/dialogs/new_peak.h"
#include "src/dialogs/new_trip.h"

#include <QFileDialog>
#include <QMessageBox>
#include <iostream>



NewAscentDialog::NewAscentDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewAscentDialog)
{
	ui->setupUi(this);
	
	connect(ui->newPeakButton,				&QPushButton::clicked,		this,	&NewAscentDialog::handle_newPeak);
	connect(ui->timeNotSpecifiedCheckbox,	&QCheckBox::stateChanged,	this,	&NewAscentDialog::handle_timeSpecifiedChanged);
	connect(ui->newTripButton,				&QPushButton::clicked,		this,	&NewAscentDialog::handle_newTrip);
	connect(ui->cancelButton,				&QPushButton::clicked,		this,	&NewAscentDialog::handle_close);
}

NewAscentDialog::~NewAscentDialog()
{
	delete ui;
}



bool NewAscentDialog::anyChanges()
{
	if (ui->peakCombo->currentIndex() > 0)				return true;
	if (ui->tripCombo->currentIndex() > 0)				return true;
	// TODO hiker
	if (!ui->descriptionEditor->document()->isEmpty())	return true;
	// TODO photos
	return false;
}



void NewAscentDialog::handle_newPeak()
{
	NewPeakDialog dialog(this);
    dialog.exec();
}

void NewAscentDialog::handle_timeSpecifiedChanged()
{
	bool enabled = !ui->timeNotSpecifiedCheckbox->isChecked();
	ui->timeWidget->setEnabled(enabled);
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
//	ui->photosPathTextfield->setText(path);
//	ui->photosPathTextfield->setFocus();
//	ui->photosPathTextfield->setCursorPosition(path.length());
}

void NewAscentDialog::reject()
{
    handle_close();
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
