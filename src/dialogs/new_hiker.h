#ifndef NEW_HIKER_H
#define NEW_HIKER_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/hiker.h"
#include "ui_new_hiker.h"



class NewHikerDialog : public NewOrEditDialog, public Ui_NewHikerDialog
{
	Q_OBJECT
	
public:
	NewHikerDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_ok();
};



Hiker* openNewHikerDialogAndStore(QWidget* parent);
bool openEditHikerDialog(QWidget* parent, Hiker* hiker);



#endif // NEW_HIKER_H
