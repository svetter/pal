#ifndef TRIP_DIALOG_H
#define TRIP_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/trip.h"
#include "ui_trip_dialog.h"



class TripDialog : public NewOrEditDialog, public Ui_TripDialog
{
	Q_OBJECT
	
	const Trip* init;
	
public:
	TripDialog(QWidget* parent, Database* db, Trip* init = nullptr);
	~TripDialog();
	
	Trip* extractData();
	virtual bool changesMade();
	
private:
	void insertInitData();
	
	void handle_datesSpecifiedChanged();	
	void handle_startDateChanged();	
	void handle_endDateChanged();
	
	void handle_ok();
};



int openNewTripDialogAndStore(QWidget* parent, Database* db);
void openEditTripDialogAndStore(QWidget* parent, Database* db, Trip* originalTrip);



#endif // TRIP_DIALOG_H
