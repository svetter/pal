#ifndef NEW_HIKER_H
#define NEW_HIKER_H

#include "src/data/hiker.h"
#include "ui_new_hiker.h"



class NewHikerDialog : public QDialog, public Ui_NewHikerDialog
{
	Q_OBJECT
	
public:
	NewHikerDialog(QWidget *parent = 0);
	~NewHikerDialog();
	
private:
	Ui::NewHikerDialog *ui;
	void reject();
	
	bool anyChanges();
	
	void handle_close();
};



Hiker* openNewHikerDialog();
bool openEditHikerDialog(Hiker* hiker);



#endif // NEW_HIKER_H
