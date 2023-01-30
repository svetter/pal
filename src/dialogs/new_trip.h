#ifndef NEW_TRIP_H
#define NEW_TRIP_H

#include "ui_new_trip.h"



class NewTripDialog : public QDialog, public Ui_NewTripDialog
{
	Q_OBJECT
	
public:
	NewTripDialog(QWidget *parent = 0);
	~NewTripDialog();
	
private:
	Ui::NewTripDialog *ui;
	void reject();
	
	bool anyChanges();
	
	void handle_close();
};



#endif // NEW_TRIP_H
