#ifndef NEW_RANGE_H
#define NEW_RANGE_H

#include "src/data/range.h"
#include "ui_new_range.h"



class NewRangeDialog : public QDialog, public Ui_NewRangeDialog
{
	Q_OBJECT
	
public:
	NewRangeDialog(QWidget *parent = 0);
	
private:
	void reject();
	
	bool anyChanges();
	
	void handle_ok();
	void handle_cancel();
};



Range* openNewRangeDialogAndStore(QWidget *parent);
bool openEditRangeDialog(QWidget *parent, Range* range);



#endif // NEW_RANGE_H
