#ifndef NEW_OR_EDIT_DIALOG_H
#define NEW_OR_EDIT_DIALOG_H

#include "src/db/db_interface.h"

#include <QDialog>



class NewOrEditDialog : public QDialog
{
	Q_OBJECT
	
protected:
	Database* db;
	
	bool edit;
	
	NewOrEditDialog(QWidget* parent, Database* db, bool edit);
	
	void changeStringsForEdit(QPushButton* okButton);
	
	virtual bool changesMade();
	
	void handle_cancel();
	
	void reject();
	
public:
	QString itemNameLowercase;
};



#endif // NEW_OR_EDIT_DIALOG_H
