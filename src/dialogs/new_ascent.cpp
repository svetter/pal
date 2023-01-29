#include "new_ascent.h"
#include "src/dialogs/new_peak.h"
#include "src/dialogs/new_trip.h"



NewAscentDialog::NewAscentDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
}

void NewAscentDialog::on_cancelButton_clicked()
{
    close();
}

void NewAscentDialog::on_timeNotSpecifiedCheckbox_stateChanged(int arg1)
{
    timeWidget->setEnabled(!arg1);
}

void NewAscentDialog::on_newPeakButton_clicked()
{
	NewPeakDialog dialog(this);
    dialog.exec();
}


void NewAscentDialog::on_newTripButton_clicked()
{
	NewTripDialog dialog(this);
    dialog.exec();
}

