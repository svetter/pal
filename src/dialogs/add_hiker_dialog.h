#ifndef ADD_HIKER_DIALOG_H
#define ADD_HIKER_DIALOG_H

#include "src/db/db_interface.h"
#include "ui_add_hiker_dialog.h"



class AddHikerDialog : public QDialog, public Ui_AddHikerDialog
{
	Q_OBJECT
	
	Database* db;
	
public:
	AddHikerDialog(QWidget* parent, Database* db);
	
	int extractHikerIndex();
	
private:
	void populateComboBoxes();
	
	bool changesMade();
	
	void handle_newHiker();
	
	void handle_ok();
	void handle_cancel();
};



int openAddHikerDialog(QWidget* parent, Database* db);



#endif // ADD_HIKER_DIALOG_H
