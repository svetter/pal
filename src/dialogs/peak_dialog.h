#ifndef PEAK_DIALOG_H
#define PEAK_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/peak.h"
#include "ui_peak_dialog.h"



class PeakDialog : public NewOrEditDialog, public Ui_PeakDialog
{
	Q_OBJECT
	
public:
	PeakDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_heightSpecifiedChanged();
	void handle_newRegion();
	
	void handle_ok();
};



Peak* openNewPeakDialogAndStore(QWidget* parent);
bool openEditPeakDialog(QWidget* parent, Peak* peak);



#endif // PEAK_DIALOG_H
