#ifndef RANGE_DIALOG_H
#define RANGE_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/range.h"
#include "ui_range_dialog.h"



class RangeDialog : public NewOrEditDialog, public Ui_RangeDialog
{
	Q_OBJECT
	
	const Range* init;
	
public:
	RangeDialog(QWidget* parent, Database* db, DialogPurpose purpose, Range* init);
	~RangeDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Range* extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



int openNewRangeDialogAndStore		(QWidget* parent, Database* db);
void openEditRangeDialogAndStore	(QWidget* parent, Database* db, int bufferRowIndex);
void openDeleteRangeDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex);



#endif // RANGE_DIALOG_H
