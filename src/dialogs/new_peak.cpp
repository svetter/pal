#include "new_peak.h"
#include "src/dialogs/new_range.h"



NewPeakDialog::NewPeakDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
}

void NewPeakDialog::on_cancelButton_clicked()
{
    close();
}

void NewPeakDialog::on_newRangeButton_clicked()
{
	NewRangeDialog dialog(this);
    dialog.exec();
}

