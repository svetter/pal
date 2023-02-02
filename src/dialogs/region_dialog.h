#ifndef REGION_DIALOG_H
#define REGION_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/region.h"
#include "ui_region_dialog.h"



class RegionDialog : public NewOrEditDialog, public Ui_RegionDialog
{
	Q_OBJECT
	
public:
	RegionDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_newRange();	
	void handle_newCountry();
	
	void handle_ok();
};



Region* openNewRegionDialogAndStore(QWidget* parent);
bool openEditRegionDialog(QWidget* parent, Region* region);



#endif // REGION_DIALOG_H
