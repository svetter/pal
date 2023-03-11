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
	TripDialog(QWidget* parent, Database* db, DialogPurpose purpose, Trip* init);
	~TripDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Trip* extractData();
	virtual bool changesMade() override;
	
private:
	void insertInitData();
	
private slots:
	void handle_datesSpecifiedChanged();
	void handle_startDateChanged();
	void handle_endDateChanged();
	
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



int openNewTripDialogAndStore		(QWidget* parent, Database* db);
void openEditTripDialogAndStore		(QWidget* parent, Database* db, int bufferRowIndex);
void openDeleteTripDialogAndExecute	(QWidget* parent, Database* db, int bufferRowIndex);



#endif // TRIP_DIALOG_H
