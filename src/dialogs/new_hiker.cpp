#include "new_hiker.h"



NewHikerDialog::NewHikerDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
}

void NewHikerDialog::on_cancelButton_clicked()
{
    close();
}
