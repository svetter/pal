#ifndef NEW_RANGE_H
#define NEW_RANGE_H

#include "ui_new_range.h"



class NewRangeDialog : public QDialog, public Ui_NewRangeDialog
{
	Q_OBJECT
	
public:
	NewRangeDialog(QWidget *parent = 0);
	~NewRangeDialog();
	
private:
	Ui::NewRangeDialog *ui;
	void reject();
	
	bool anyChanges();
	
	void handle_close();
};



public Range* openNewRangeDialog();
public bool openEditRangeDialog(Range* range);



#endif // NEW_RANGE_H
