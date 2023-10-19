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

#ifndef ITEM_DIALOG_H
#define ITEM_DIALOG_H

#include "src/db/database.h"

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>



enum DialogPurpose {
	newItem,
	editItem,
	duplicateItem
};



class ItemDialog : public QDialog
{
	Q_OBJECT
	
protected:
	QWidget* parent;
	
	Database* db;
	DialogPurpose purpose;
	
	ItemDialog(QWidget* parent, Database* db, DialogPurpose purpose);
	
	virtual QString getEditWindowTitle() = 0;
	
	void changeStringsForEdit(QPushButton* okButton);
	
	void handle_ok(QLineEdit* nameLineEdit, QString initName, QString emptyNameWindowTitle, QString emptyNameMessage, const Column* nameColumn);
	virtual void handle_ok() = 0;
	virtual void handle_cancel();
	void reject() override;
	
	virtual void aboutToClose() = 0;
	
	bool checkNameForDuplicatesAndWarn(QString name, const Column* nameColumn);
	
public:
	virtual bool changesMade() = 0;
};



bool displayDeleteWarning(QWidget* parent, QString windowTitle, const QList<WhatIfDeleteResult>& whatIfResults);

void populateItemCombo(NormalTable* table, const Column* displayAndSortColumn, bool sortAsString, QComboBox* combo, QList<ValidItemID>& idList, QString overrideFirstLine = QString(), const Column* filterColumn = nullptr, ItemID filterID = ItemID());



#endif // ITEM_DIALOG_H
