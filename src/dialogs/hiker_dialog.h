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
 * @file hiker_dialog.h
 * 
 * This file declares the HikerDialog class.
 */

#ifndef HIKER_DIALOG_H
#define HIKER_DIALOG_H

#include "src/db/database.h"
#include "src/dialogs/item_dialog.h"
#include "src/data/hiker.h"
#include "ui_hiker_dialog.h"

using std::unique_ptr;



/**
 * Control class for the hiker dialog.
 */
class HikerDialog : public ItemDialog, public Ui_HikerDialog
{
	Q_OBJECT
	
	/** The hiker data before user interaction starts. */
	unique_ptr<const Hiker> init;
	
public:
	HikerDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Hiker> init);
	~HikerDialog();
	
	virtual QString getEditWindowTitle() override;
	
	unique_ptr<Hiker> extractData();
	virtual bool changesMade() override;
	
private:
	void insertInitData();
	
private slots:
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewHikerDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db);
bool			openEditHikerDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex);
bool			openDeleteHikersDialogAndExecute	(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices);

BufferRowIndex	openHikerDialogAndStore				(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<Hiker> originalHiker);



#endif // HIKER_DIALOG_H
