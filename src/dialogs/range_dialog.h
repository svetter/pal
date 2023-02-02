#ifndef RANGE_DIALOG_H
#define RANGE_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/range.h"
#include "ui_range_dialog.h"



class RangeDialog : public NewOrEditDialog, public Ui_RangeDialog
{
	Q_OBJECT
	
	Range* init;
	
public:
	RangeDialog(QWidget* parent, Range* init = nullptr);
	
private:
	void insertInitData();
	
	virtual bool changesMade();
	
	void handle_ok();
};



Range* openNewRangeDialogAndStore(QWidget* parent);
bool openEditRangeDialog(QWidget* parent, Range* range);



#endif // RANGE_DIALOG_H
