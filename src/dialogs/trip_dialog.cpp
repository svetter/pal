#include "trip_dialog.h"

#include <QMessageBox>



TripDialog::TripDialog(QWidget* parent, Trip* init) :
		NewOrEditDialog(parent, tr("trip"), init != nullptr),
		init(init)
{
	setupUi(this);
	
	
	connect(datesUnspecifiedCheckbox,	&QCheckBox::stateChanged,	this,	&TripDialog::handle_datesSpecifiedChanged);
	
	connect(okButton,					&QPushButton::clicked,		this,	&TripDialog::handle_ok);
	connect(cancelButton,				&QPushButton::clicked,		this,	&TripDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}



void TripDialog::insertInitData()
{
	if (!init) return;
	// TODO
}



bool TripDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())				return true;
	if (!descriptionEditor->document()->isEmpty())	return true;
	return false;
}



void TripDialog::handle_datesSpecifiedChanged()
{
	bool enabled = !datesUnspecifiedCheckbox->isChecked();
	startDateWidget->setEnabled(enabled);
	endDateWidget->setEnabled(enabled);
}



void TripDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new trip");
		QString message = tr("The trip needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Trip* openNewTripDialogAndStore(QWidget* parent)
{
	TripDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Trip* trip = new Trip();
		trip->name = dialog.nameTextbox->text();
		// TODO
		return trip;
	}
	return nullptr;
}

bool openEditTripDialog(QWidget* parent, Trip* trip)
{
	TripDialog dialog(parent, trip);
	dialog.exec();
	// TODO
	return false;
}
