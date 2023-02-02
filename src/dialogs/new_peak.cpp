#include "new_peak.h"
#include "src/dialogs/new_region.h"

#include <QMessageBox>



NewPeakDialog::NewPeakDialog(QWidget* parent): NewOrEditDialog(parent, tr("peak"))
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(heightCheckbox,		&QCheckBox::stateChanged,	this,	&NewPeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,	&QPushButton::clicked,		this,	&NewPeakDialog::handle_newRegion);
	
	connect(okButton,			&QPushButton::clicked,		this,	&NewPeakDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,		this,	&NewPeakDialog::handle_cancel);	
}



bool NewPeakDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())			return true;
	if (regionCombo->currentIndex() > 0)		return true;
	if (volcanoCheckbox->isChecked())			return true;
	if (!googleMapsTextbox->text().isEmpty())	return true;
	if (!googleEarthTextbox->text().isEmpty())	return true;
	if (!wikipediaTextbox->text().isEmpty())	return true;
	return false;
}



void NewPeakDialog::handle_heightSpecifiedChanged()
{
	bool enabled = heightCheckbox->isChecked();
	heightSpinner->setEnabled(enabled);
}

void NewPeakDialog::handle_newRegion()
{
	openNewRegionDialogAndStore(this);
	// TODO
}



void NewPeakDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new peak");
		QString message = tr("The peak needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Peak* openNewPeakDialogAndStore(QWidget* parent)
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

bool openEditPeakDialog(QWidget* parent, Peak* peak)
{
	// TODO
	return false;
}
