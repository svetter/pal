#ifndef NEW_OR_EDIT_DIALOG_H
#define NEW_OR_EDIT_DIALOG_H

#include "src/db/database.h"

#include <QDialog>



enum DialogPurpose {
	newItem,
	editItem,
	duplicateItem
};



class NewOrEditDialog : public QDialog
{
	Q_OBJECT
	
protected:
	Database* db;
	DialogPurpose purpose;
	
	NewOrEditDialog(QWidget* parent, Database* db, DialogPurpose purpose);
	
	virtual QString getEditWindowTitle() = 0;
	
	void changeStringsForEdit(QPushButton* okButton);
	
	void handle_cancel();
	void reject() override;
	
public:
	virtual bool changesMade() = 0;
};



bool displayDeleteWarning(QWidget* parent, QString windowTitle, const QList<WhatIfDeleteResult>& whatIfResults);



#endif // NEW_OR_EDIT_DIALOG_H
