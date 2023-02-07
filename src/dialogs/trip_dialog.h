#ifndef TRIP_DIALOG_H
#define TRIP_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/trip.h"
#include "ui_trip_dialog.h"



class TripDialog : public NewOrEditDialog, public Ui_TripDialog
{
	Q_OBJECT
	
	Trip* init;
	
public:
	TripDialog(QWidget* parent, Database* db, Trip* init = nullptr);
	~TripDialog();
	
	Trip* extractData();
	
private:
	void insertInitData();
	virtual bool changesMade();
	
	void handle_datesSpecifiedChanged();
	
	void handle_ok();
};



Trip* openNewTripDialogAndStore(QWidget* parent, Database* db);
bool openEditTripDialog(QWidget* parent, Database* db, Trip* trip);



#endif // TRIP_DIALOG_H
