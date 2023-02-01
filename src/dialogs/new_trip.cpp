#include "new_trip.h"

#include <QMessageBox>



NewTripDialog::NewTripDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	
	connect(datesUnspecifiedCheckbox,	&QCheckBox::stateChanged,	this,	&NewTripDialog::handle_datesSpecifiedChanged);
	
	connect(okButton,					&QPushButton::clicked,		this,	&QDialog::accept);
	connect(cancelButton,				&QPushButton::clicked,		this,	&NewTripDialog::handle_close);
}



bool NewTripDialog::anyChanges()
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

void NewTripDialog::handle_close()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new trip");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewTripDialog::reject()
{
	handle_close();
}



Trip* openNewTripDialogAndStore(QWidget *parent)
{
	NewTripDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		QString name = dialog.nameTextbox->text();
		// TODO
		//return new Trip(...);
	}
	return nullptr;
}

bool openEditTripDialog(QWidget *parent, Trip* trip)
{
	// TODO
	return false;
}
