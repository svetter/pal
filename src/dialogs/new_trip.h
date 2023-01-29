#ifndef NEW_TRIP_H
#define NEW_TRIP_H

#include "ui_new_trip.h"



class NewTripDialog : public QDialog, public Ui_NewTripDialog
{
	Q_OBJECT
public:
	NewTripDialog(QWidget *parent = 0);
private slots:
	void on_cancelButton_clicked();
};



#endif // NEW_TRIP_H
