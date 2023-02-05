#ifndef REGION_DIALOG_H
#define REGION_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/region.h"
#include "ui_region_dialog.h"



class RegionDialog : public NewOrEditDialog, public Ui_RegionDialog
{
	Q_OBJECT
	
	Region* init;
	
public:
	RegionDialog(QWidget* parent, Database* db, Region* init = nullptr);
	
private:
	void insertInitData();
	
	virtual bool changesMade();
	
	void handle_newRange();	
	void handle_newCountry();
	
	void handle_ok();
};



Region* openNewRegionDialogAndStore(QWidget* parent, Database* db);
bool openEditRegionDialog(QWidget* parent, Database* db, Region* region);



#endif // REGION_DIALOG_H
