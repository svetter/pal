#ifndef NEW_REGION_H
#define NEW_REGION_H

#include "ui_new_region.h"



class NewRegionDialog : public QDialog, public Ui_NewRegionDialog
{
	Q_OBJECT
public:
	NewRegionDialog(QWidget *parent = 0);
private slots:
	void on_cancelButton_clicked();
};



#endif // NEW_REGION_H
