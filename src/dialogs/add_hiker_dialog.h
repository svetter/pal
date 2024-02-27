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
 * @file add_hiker_dialog.h
 * 
 * This file declates the AddHikerDialog class.
 */

#ifndef ADD_HIKER_DIALOG_H
#define ADD_HIKER_DIALOG_H

#include "src/db/database.h"
#include "ui_add_hiker_dialog.h"

#include <QMainWindow>



/**
 * Control class for the add hiker dialog used in the ascent dialog to add a hiker to the ascent.
 */
class AddHikerDialog : public QDialog, public Ui_AddHikerDialog
{
	Q_OBJECT
	
	/** The application's main window. */
	QMainWindow& mainWindow;
	/** The project database. */
	Database& db;
	
	/** The list of hiker IDs in the order they appear in the hiker combo box. */
	QList<ValidItemID> selectableHikerIDs;
	
public:
	AddHikerDialog(QWidget& parent, QMainWindow& mainWindow, Database& db);
	
private:
	void populateComboBoxes();
	
public:
	ValidItemID extractHikerID();
private:
	bool hikerSelected();
	
	void handle_newHiker();
	
	void handle_ok();
	void handle_cancel();
};



ItemID openAddHikerDialog(QWidget& parent, QMainWindow& mainWindow, Database& db);



#endif // ADD_HIKER_DIALOG_H
