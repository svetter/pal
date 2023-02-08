#ifndef HIKER_DIALOG_H
#define HIKER_DIALOG_H

#include "src/db/db_interface.h"
#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/hiker.h"
#include "ui_hiker_dialog.h"



class HikerDialog : public NewOrEditDialog, public Ui_HikerDialog
{
	Q_OBJECT
	
	Hiker* init;
	
public:
	HikerDialog(QWidget* parent, Database* db, Hiker* init = nullptr);
	~HikerDialog();
	
	Hiker* extractData();
	virtual bool changesMade();
	
private:
	void insertInitData();
	
	void handle_ok();
};



Hiker* openNewHikerDialogAndStore(QWidget* parent, Database* db);
Hiker* openEditHikerDialog(QWidget* parent, Database* db, Hiker* originalHiker);



#endif // HIKER_DIALOG_H
