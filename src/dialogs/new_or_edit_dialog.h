#ifndef NEW_OR_EDIT_DIALOG_H
#define NEW_OR_EDIT_DIALOG_H

#include "src/db/db_interface.h"

#include <QDialog>



class NewOrEditDialog : public QDialog
{
	Q_OBJECT
	
protected:
	QString editWindowTitle;
	
	Database* db;
	
	bool edit;
	
	NewOrEditDialog(QWidget* parent, Database* db, bool edit, QString editWindowTitle);
	
	void changeStringsForEdit(QPushButton* okButton);
	
	void handle_cancel();
	
	void reject();
	
public:
	virtual bool changesMade() = 0;
};



#endif // NEW_OR_EDIT_DIALOG_H
