#ifndef NEW_TRIP_H
#define NEW_TRIP_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/trip.h"
#include "ui_new_trip.h"



class NewTripDialog : public NewOrEditDialog, public Ui_NewTripDialog
{
	Q_OBJECT
	
public:
	NewTripDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_datesSpecifiedChanged();
	
	void handle_ok();
};



Trip* openNewTripDialogAndStore(QWidget *parent);
bool openEditTripDialog(QWidget *parent, Trip* trip);



#endif // NEW_TRIP_H
