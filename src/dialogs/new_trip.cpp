#include "new_trip.h"



NewTripDialog::NewTripDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
}

void NewTripDialog::on_cancelButton_clicked()
{
    close();
}
