#ifndef ADD_HIKER_H
#define ADD_HIKER_H

#include "ui_new_hiker.h"



class NewHikerDialog : public QWidget, public Ui_AddHikerDialog
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



#endif // ADD_HIKER_H
