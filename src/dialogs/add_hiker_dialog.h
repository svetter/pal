#ifndef HIKER_DIALOG_H
#define HIKER_DIALOG_H

#include "ui_add_hiker_dialog.h"



class AddHikerDialog : public QWidget, public Ui_AddHikerDialog
{
	Q_OBJECT
	
public:
	AddHikerDialog(QWidget* parent);
	
private:
	bool changesMade();
	
	void handle_ok();
	void handle_cancel();
};



int openAddHikerDialog(QWidget* parent);



#endif // HIKER_DIALOG_H
