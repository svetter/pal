#ifndef NEW_RANGE_H
#define NEW_RANGE_H

#include "ui_new_range.h"



class NewRangeDialog : public QDialog, public Ui_NewRangeDialog
{
	Q_OBJECT
public:
	NewRangeDialog(QWidget *parent = 0);
private slots:
	void on_cancelButton_clicked();
};



#endif // NEW_RANGE_H
