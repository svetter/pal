#include "new_trip.h"

#include <QMessageBox>



NewTripDialog::NewTripDialog(QWidget *parent): NewOrEditDialog(parent, tr("trip"))
{
	setupUi(this);
	
	connect(datesUnspecifiedCheckbox,	&QCheckBox::stateChanged,	this,	&NewTripDialog::handle_datesSpecifiedChanged);
	
	connect(okButton,					&QPushButton::clicked,		this,	&NewTripDialog::handle_ok);
	connect(cancelButton,				&QPushButton::clicked,		this,	&NewTripDialog::handle_cancel);
}



bool NewTripDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())				return true;
	if (!descriptionEditor->document()->isEmpty())	return true;
	return false;
}



void NewTripDialog::handle_datesSpecifiedChanged()
{
	bool enabled = !datesUnspecifiedCheckbox->isChecked();
	startDateWidget->setEnabled(enabled);
	endDateWidget->setEnabled(enabled);
}



void NewTripDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new trip");
		QString question = tr("The trip needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, question, ok, ok);
	}
}



Trip* openNewTripDialogAndStore(QWidget *parent)
{
	NewTripDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Trip* trip = new Trip();
		trip->name = dialog.nameTextbox->text();
		// TODO
		return trip;
	}
	return nullptr;
}

bool openEditTripDialog(QWidget *parent, Trip* trip)
{
	// TODO
	return false;
}
