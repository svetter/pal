#include "new_country.h"



NewCountryDialog::NewCountryDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
}

void NewCountryDialog::on_cancelButton_clicked()
{
    close();
}
