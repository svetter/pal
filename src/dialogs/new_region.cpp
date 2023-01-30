#include "new_region.h"



NewRegionDialog::NewRegionDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
}

void NewRegionDialog::on_cancelButton_clicked()
{
    close();
}
