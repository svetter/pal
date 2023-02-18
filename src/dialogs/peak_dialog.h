#ifndef PEAK_DIALOG_H
#define PEAK_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/peak.h"
#include "ui_peak_dialog.h"



class PeakDialog : public NewOrEditDialog, public Ui_PeakDialog
{
	Q_OBJECT
	
	const Peak* init;
	
public:
	PeakDialog(QWidget* parent, Database* db, Peak* init = nullptr);
	~PeakDialog();
	
	Peak* extractData();
	virtual bool changesMade();
	
private:
	void populateComboBoxes();
	void insertInitData();
	
	void handle_heightSpecifiedChanged();
	void handle_newRegion();
	
	void handle_ok();
};



int openNewPeakDialogAndStore(QWidget* parent, Database* db);
int openNewPeakDialogAndStore(QWidget* parent, Database* db, Peak* copyFrom);
void openEditPeakDialogAndStore(QWidget* parent, Database* db, Peak* originalPeak);



#endif // PEAK_DIALOG_H
