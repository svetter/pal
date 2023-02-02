#ifndef TRIP_DIALOG_H
#define TRIP_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/trip.h"
#include "ui_trip_dialog.h"



class TripDialog : public NewOrEditDialog, public Ui_TripDialog
{
	Q_OBJECT
	
public:
	TripDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_datesSpecifiedChanged();
	
	void handle_ok();
};



Trip* openNewTripDialogAndStore(QWidget* parent);
bool openEditTripDialog(QWidget* parent, Trip* trip);



#endif // TRIP_DIALOG_H
