#include "new_peak.h"
#include "src/dialogs/new_region.h"

#include <QMessageBox>



NewPeakDialog::NewPeakDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(heightCheckbox,		&QCheckBox::stateChanged,	this,	&NewPeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,	&QPushButton::clicked,		this,	&NewPeakDialog::handle_newRegion);
	
	connect(okButton,			&QPushButton::clicked,		this,	&NewPeakDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,		this,	&NewPeakDialog::handle_cancel);	
}



bool NewPeakDialog::anyChanges()
{
	if (!nameTextbox->text().isEmpty())			return true;
	if (regionCombo->currentIndex() > 0)		return true;
	if (volcanoCheckbox->isChecked())			return true;
	if (!googleMapsTextbox->text().isEmpty())	return true;
	if (!googleEarthTextbox->text().isEmpty())	return true;
	if (!wikipediaTextbox->text().isEmpty())	return true;
	return false;
}



void NewPeakDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new peak");
		QString question = tr("The peak needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, question, ok, ok);
	}
}

void NewPeakDialog::handle_heightSpecifiedChanged()
{
	bool enabled = heightCheckbox->isChecked();
	heightSpinner->setEnabled(enabled);
}

void NewPeakDialog::handle_newRegion()
{
	NewRegionDialog dialog(this);
	dialog.exec();
}

void NewPeakDialog::handle_cancel()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new peak");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewPeakDialog::reject()
{
	handle_cancel();
}



Peak* openNewPeakDialogAndStore(QWidget *parent)
{
	NewPeakDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Peak* peak = new Peak();
		peak->name = dialog.nameTextbox->text();
		// TODO
		return peak;
	}
	return nullptr;
}

bool openEditPeakDialog(QWidget *parent, Peak* peak)
{
	// TODO
	return false;
}
