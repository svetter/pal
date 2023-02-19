#ifndef REGION_DIALOG_H
#define REGION_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/region.h"
#include "ui_region_dialog.h"



class RegionDialog : public NewOrEditDialog, public Ui_RegionDialog
{
	Q_OBJECT
	
	const Region* init;
	
public:
	RegionDialog(QWidget* parent, Database* db, DialogPurpose purpose, Region* init);
	~RegionDialog();
	
	virtual QString getEditWindowTitle();
	
	Region* extractData();
	virtual bool changesMade();
	
private:
	void populateComboBoxes();
	void insertInitData();
	
	void handle_newRange();	
	void handle_newCountry();
	
	void handle_ok();
};



int openNewRegionDialogAndStore(QWidget* parent, Database* db);
void openEditRegionDialogAndStore(QWidget* parent, Database* db, Region* originalRegion);
void openDeleteRegionDialogAndExecute(QWidget* parent, Database* db, Region* ascent);



#endif // REGION_DIALOG_H
