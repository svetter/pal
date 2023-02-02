#ifndef ADD_HIKER_DIALOG_H
#define ADD_HIKER_DIALOG_H

#include "ui_add_hiker_dialog.h"



class AddHikerDialog : public QDialog, public Ui_AddHikerDialog
{
	Q_OBJECT
	
public:
	AddHikerDialog(QWidget* parent);
	
private:
	bool changesMade();	
	
	void handle_newHiker();
	
	void handle_ok();
	void handle_cancel();
};



int openAddHikerDialog(QWidget* parent);



#endif // ADD_HIKER_DIALOG_H
