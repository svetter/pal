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

/**
 * @file item_dialog.h
 * 
 * This file declares the ItemDialog class.
 */

#ifndef ITEM_DIALOG_H
#define ITEM_DIALOG_H

#include "src/db/database.h"

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>



/**
 * The purpose of an item dialog.
 */
enum DialogPurpose {
	newItem,
	editItem,
	duplicateItem
};



/**
 * The base class for all item dialogs.
 */
class ItemDialog : public QDialog
{
	Q_OBJECT
	
protected:
	/** The parent window. */
	QWidget* parent;
	
	/** The project database. */
	Database* db;
	/** The purpose of the dialog. */
	DialogPurpose purpose;
	
	ItemDialog(QWidget* parent, Database* db, DialogPurpose purpose);
	
	/**
	 * Returns the window title to use when the dialog is used to edit an item.
	 * 
	 * @return	The window title for editing an item
	 */
	virtual QString getEditWindowTitle() = 0;
	
	void changeStringsForEdit(QPushButton* okButton);
	
	void handle_ok(QLineEdit* nameLineEdit, QString initName, QString emptyNameWindowTitle, QString emptyNameMessage, const Column* nameColumn);
	/**
	 * Event handler for the OK button.
	 */
	virtual void handle_ok() = 0;
	/**
	 * Event handler for the Cancel button.
	 */
	virtual void handle_cancel();
	void reject() override;
	
	/**
	 * Called before the dialog is closed to allow the dialog to save changes and implicit
	 * settings.
	 */
	virtual void aboutToClose() = 0;
	
	bool checkNameForDuplicatesAndWarn(QString name, const Column* nameColumn);
	
public:
	/**
	 * Checks whether changes have been made to the item.
	 * 
	 * @return	True if the item is different from its state when the dialog was opened, false otherwise.
	 */
	virtual bool changesMade() = 0;
};



bool displayDeleteWarning(QWidget* parent, QString windowTitle, const QList<WhatIfDeleteResult>& whatIfResults);

void populateItemCombo(NormalTable* table, const Column* displayAndSortColumn, bool sortAsString, QComboBox* combo, QList<ValidItemID>& idList, QString overrideFirstLine = QString(), const Column* filterColumn = nullptr, ItemID filterID = ItemID());



#endif // ITEM_DIALOG_H
