#ifndef NEW_REGION_H
#define NEW_REGION_H

#include "src/data/region.h"
#include "ui_new_region.h"



class NewRegionDialog : public QDialog, public Ui_NewRegionDialog
{
	Q_OBJECT
	
public:
	NewRegionDialog(QWidget *parent = 0);
	
private:
	void reject();
	
	bool anyChanges();
	
	void handle_newRange();	
	void handle_newCountry();
	void handle_close();
};



Region* openNewRegionDialog(QWidget *parent);
bool openEditRegionDialog(QWidget *parent, Region* region);



#endif // NEW_REGION_H
