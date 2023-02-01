#ifndef NEW_PEAK_H
#define NEW_PEAK_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/peak.h"
#include "ui_new_peak.h"



class NewPeakDialog : public NewOrEditDialog, public Ui_NewPeakDialog
{
	Q_OBJECT
	
public:
	NewPeakDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_heightSpecifiedChanged();
	void handle_newRegion();
	
	void handle_ok();
};



Peak* openNewPeakDialogAndStore(QWidget *parent);
bool openEditPeakDialog(QWidget *parent, Peak* peak);



#endif // NEW_PEAK_H
