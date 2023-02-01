#ifndef NEW_REGION_H
#define NEW_REGION_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/region.h"
#include "ui_new_region.h"



class NewRegionDialog : public NewOrEditDialog, public Ui_NewRegionDialog
{
	Q_OBJECT
	
public:
	NewRegionDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_newRange();	
	void handle_newCountry();
	
	void handle_ok();
};



Region* openNewRegionDialogAndStore(QWidget *parent);
bool openEditRegionDialog(QWidget *parent, Region* region);



#endif // NEW_REGION_H
