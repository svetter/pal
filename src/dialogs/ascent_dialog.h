#ifndef ASCENT_DIALOG_H
#define ASCENT_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/ascent.h"
#include "ui_ascent_dialog.h"

#include "list/hiker_list.h"
#include "list/photo_list.h"

#include <QWidget>



class AscentDialog : public NewOrEditDialog, public Ui_AscentDialog
{
	Q_OBJECT
	
	const Ascent* init;
	
	HikersOnAscent hikersModel;
	PhotosOfAscent photosModel;
	
public:
	AscentDialog(QWidget* parent, Database* db, Ascent* init = nullptr);
	~AscentDialog();
	
	Ascent* extractData();
	virtual bool changesMade();
	
private:
	void populateComboBoxes();
	void insertInitData();
	
	void handle_newPeak();
	void handle_dateSpecifiedChanged();
	void handle_timeSpecifiedChanged();
	void handle_difficultySystemChanged();
	void handle_newTrip();
	void handle_addHiker();
	void handle_removeHikers();
	void handle_addPhotos();
	void handle_removePhotos();
	
	void handle_ok();
};



int openNewAscentDialogAndStore(QWidget* parent, Database* db);
int openNewAscentDialogAndStore(QWidget* parent, Database* db, Ascent* copyFrom);
void openEditAscentDialogAndStore(QWidget* parent, Database* db, Ascent* originalAscent);
void openDeleteAscentDialogAndExecute(QWidget* parent, Database* db, Ascent* ascent);



#endif // ASCENT_DIALOG_H
