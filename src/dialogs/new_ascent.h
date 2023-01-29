#ifndef NEW_ASCENT_H
#define NEW_ASCENT_H

#include "ui_new_ascent.h"



class NewAscentDialog : public QDialog, public Ui_NewAscentDialog
{
	Q_OBJECT
public:
	NewAscentDialog(QWidget *parent = 0);
private slots:
	void on_cancelButton_clicked();
	void on_timeNotSpecifiedCheckbox_stateChanged(int arg1);
	void on_newPeakButton_clicked();
	void on_newTripButton_clicked();
};



#endif // NEW_ASCENT_H
