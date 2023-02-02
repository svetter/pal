#ifndef HIKER_DIALOG_H
#define HIKER_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/hiker.h"
#include "ui_hiker_dialog.h"



class HikerDialog : public NewOrEditDialog, public Ui_HikerDialog
{
	Q_OBJECT
	
	Hiker* init;
	
public:
	HikerDialog(QWidget* parent, Hiker* init = nullptr);
	
private:
	void insertInitData();
	
	virtual bool changesMade();
	
	void handle_ok();
};



Hiker* openNewHikerDialogAndStore(QWidget* parent);
bool openEditHikerDialog(QWidget* parent, Hiker* hiker);



#endif // HIKER_DIALOG_H
