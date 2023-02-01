#ifndef NEW_OR_EDIT_DIALOG_H
#define NEW_OR_EDIT_DIALOG_H

#include "qdialog.h"



class NewOrEditDialog : public QDialog
{
	Q_OBJECT
	
protected:
	NewOrEditDialog(QWidget* parent, QString itemNameLowercase);
	
	void reject();
	
	virtual bool changesMade();
	
	void handle_cancel();
	
public:
	QString itemNameLowercase;
};



#endif // NEW_OR_EDIT_DIALOG_H
