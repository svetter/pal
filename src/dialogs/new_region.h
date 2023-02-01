#ifndef NEW_REGION_H
#define NEW_REGION_H

#include "ui_new_region.h"



class NewRegionDialog : public QDialog, public Ui_NewRegionDialog
{
	Q_OBJECT
	
public:
	NewRegionDialog(QWidget *parent = 0);
	~NewRegionDialog();
	
private:
	Ui::NewRegionDialog *ui;
	void reject();
	
	bool anyChanges();
	
	void handle_newRange();	
	void handle_newCountry();
	void handle_close();
};



public Region* openNewRegionDialog();
public bool openEditRegionDialog(Region* region);



#endif // NEW_REGION_H
