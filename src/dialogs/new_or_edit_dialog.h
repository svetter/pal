#ifndef NEW_OR_EDIT_DIALOG_H
#define NEW_OR_EDIT_DIALOG_H

#include "src/db/database.h"

#include <QDialog>
#include <QComboBox>



enum DialogPurpose {
	newItem,
	editItem,
	duplicateItem
};



class NewOrEditDialog : public QDialog
{
	Q_OBJECT
	
protected:
	QWidget* parent;
	
	Database* db;
	DialogPurpose purpose;
	
	NewOrEditDialog(QWidget* parent, Database* db, DialogPurpose purpose);
	
	virtual QString getEditWindowTitle() = 0;
	
	void changeStringsForEdit(QPushButton* okButton);
	
	virtual void handle_cancel();
	void reject() override;
	
	virtual void aboutToClose() = 0;
	
public:
	virtual bool changesMade() = 0;
};



bool displayDeleteWarning(QWidget* parent, QString windowTitle, const QList<WhatIfDeleteResult>& whatIfResults);

void populateItemCombo(NormalTable* table, const Column* displayAndSortColumn, QComboBox* combo, QList<ValidItemID>& idList);



#endif // NEW_OR_EDIT_DIALOG_H
