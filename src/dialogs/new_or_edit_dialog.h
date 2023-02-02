#ifndef NEW_OR_EDIT_DIALOG_H
#define NEW_OR_EDIT_DIALOG_H

#include "qdialog.h"



class NewOrEditDialog : public QDialog
{
	Q_OBJECT
	
	QString s;
	
protected:
	bool edit;
	
	NewOrEditDialog(QWidget* parent, QString itemNameLowercase, bool edit);
	
	void changeStringsForEdit(QPushButton* okButton);
	
	virtual bool changesMade();
	
	void handle_cancel();
	
	void reject();
	
public:
	QString itemNameLowercase;
};



#endif // NEW_OR_EDIT_DIALOG_H
