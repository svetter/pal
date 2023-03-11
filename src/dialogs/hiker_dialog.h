#ifndef HIKER_DIALOG_H
#define HIKER_DIALOG_H

#include "src/db/database.h"
#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/hiker.h"
#include "ui_hiker_dialog.h"



class HikerDialog : public NewOrEditDialog, public Ui_HikerDialog
{
	Q_OBJECT
	
	const Hiker* init;
	
public:
	HikerDialog(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* init);
	~HikerDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Hiker* extractData();
	virtual bool changesMade() override;
	
private:
	void insertInitData();
	
private slots:
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



int openNewHikerDialogAndStore		(QWidget* parent, Database* db);
void openEditHikerDialogAndStore	(QWidget* parent, Database* db, int bufferRowIndex);
void openDeleteHikerDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex);



#endif // HIKER_DIALOG_H
