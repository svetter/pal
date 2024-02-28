/*
 * Copyright 2023-2024 Simon Vetter
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
#include <QCheckBox>
#include <QPushButton>
#include <QMainWindow>



/**
 * The purpose of an item dialog.
 */
enum DialogPurpose {
	newItem,
	editItem,
	multiEdit,
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
	QWidget& parent;
	/** The application's main window. */
	QMainWindow& mainWindow;
	
	/** The project database. */
	Database& db;
	/** The purpose of the dialog. */
	const DialogPurpose purpose;
	
private:
	/** The dialog's save button. */
	QPushButton* saveButton;
	/** The checkboxes which control which values are edited when the dialog is used for multi-editing, along with all widgets that are affected by each checkbox and the corresponding column in the item table. */
	QMap<QCheckBox*, QPair<QSet<QWidget*>, QSet<const Column*>>> multiEditCheckboxes;
	/** The checkboxes which need to be turned into tristate checkboxes when editing multiple items, along with the corresponding column in the item table. */
	QMap<QCheckBox*, QSet<const Column*>> tristateCheckboxes;
	
	/** The previous enable states of all widgets currently disabled because of a multi-edit checkbox. */
	QMap<QCheckBox*, QMap<QWidget*, bool>> savedWidgetEnabledStates;
	
protected:
	ItemDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, const QString& windowTitle);
	
	void setUIPointers(QPushButton* saveButton, const QMap<QCheckBox*, QPair<QSet<QWidget*>, QSet<const Column*>>>& multiEditCheckboxes, const QMap<QCheckBox*, QSet<const Column*>>& tristateCheckboxes = QMap<QCheckBox*, QSet<const Column*>>());
	void changeUIForPurpose();
	void handle_multiEditCheckboxClicked();
	bool anyMultiEditChanges();
public:
	QSet<const Column*> getMultiEditColumns();
	
protected:
	void handle_ok(QLineEdit* nameLineEdit, QString initName, QString emptyNameWindowTitle, QString emptyNameMessage, const ValueColumn& nameColumn);
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
	
	bool checkNameForDuplicatesAndWarn(QString name, const ValueColumn& nameColumn);
	
public:
	/**
	 * Checks whether changes have been made to the item.
	 * 
	 * @return	True if the item is different from its state when the dialog was opened, false otherwise.
	 */
	virtual bool changesMade() = 0;
};



bool displayDeleteWarning(QWidget& parent, const QString& windowTitle, const QList<WhatIfDeleteResult>& whatIfResults);



void populateItemCombo(QComboBox& combo, const ValueColumn& displayAndSortColumn, QList<ValidItemID>& idList, const QString& overrideFirstLine = QString(), const ForeignKeyColumn* distinctionKeyColumn = nullptr, const ValueColumn* distinctionContentColumn = nullptr, const ForeignKeyColumn* filterColumn = nullptr, ItemID filterID = ItemID(), const ValueColumn* prefixColumn = nullptr, std::function<QString (const QVariant&)> prefixValueToString = nullptr);

void populatePeakCombo		(Database& db, QComboBox& peakCombo,	QList<ValidItemID>& selectablePeakIDs,		ItemID regionFilterID = ItemID());
void populateTripCombo		(Database& db, QComboBox& tripCombo,	QList<ValidItemID>& selectableTripIDs);
void populateHikerCombo		(Database& db, QComboBox& hikerCombo,	QList<ValidItemID>& selectableHikerIDs);
void populateRegionCombo	(Database& db, QComboBox& regionCombo,	QList<ValidItemID>& selectableRegionIDs,	bool asFilter = false);
void populateRangeCombo		(Database& db, QComboBox& rangeCombo,	QList<ValidItemID>& selectableRangeIDs);
void populateCountryCombo	(Database& db, QComboBox& countryCombo,	QList<ValidItemID>& selectableCountryIDs);



#endif // ITEM_DIALOG_H
