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
	
	void handle_datesSpecifiedChanged();
	void handle_close();
};



public Trip* openNewTripDialog();
public bool openEditTripDialog(Trip* trip);



#endif // NEW_TRIP_H
