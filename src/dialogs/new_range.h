#ifndef NEW_RANGE_H
#define NEW_RANGE_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/range.h"
#include "ui_new_range.h"



class NewRangeDialog : public NewOrEditDialog, public Ui_NewRangeDialog
{
	Q_OBJECT
	
public:
	NewRangeDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_ok();
};



Range* openNewRangeDialogAndStore(QWidget *parent);
bool openEditRangeDialog(QWidget *parent, Range* range);



#endif // NEW_RANGE_H
