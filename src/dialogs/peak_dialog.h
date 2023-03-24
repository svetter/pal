#ifndef PEAK_DIALOG_H
#define PEAK_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/peak.h"
#include "ui_peak_dialog.h"



class PeakDialog : public NewOrEditDialog, public Ui_PeakDialog
{
	Q_OBJECT
	
	const Peak* init;
	
	QList<ValidItemID> selectableRegionIDs;
	
public:
	PeakDialog(QWidget* parent, Database* db, DialogPurpose purpose, Peak* init);
	~PeakDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Peak* extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_heightSpecifiedChanged();
	void handle_newRegion();
	
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



int openNewPeakDialogAndStore		(QWidget* parent, Database* db);
int openDuplicatePeakDialogAndStore	(QWidget* parent, Database* db, int bufferRowIndex);
void openEditPeakDialogAndStore		(QWidget* parent, Database* db, int bufferRowIndex);
void openDeletePeakDialogAndExecute	(QWidget* parent, Database* db, int bufferRowIndex);



#endif // PEAK_DIALOG_H
