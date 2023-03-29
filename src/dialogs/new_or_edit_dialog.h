/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

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

void populateItemCombo(NormalTable* table, const Column* displayAndSortColumn, bool sortAsString, QComboBox* combo, QList<ValidItemID>& idList);



#endif // NEW_OR_EDIT_DIALOG_H
