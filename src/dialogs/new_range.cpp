#include "new_range.h"



NewRangeDialog::NewRangeDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
}

void NewRangeDialog::on_cancelButton_clicked()
{
    close();
}
