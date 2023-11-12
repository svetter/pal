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



/**
 * Control class for the hiker dialog.
 */
class HikerDialog : public ItemDialog, public Ui_HikerDialog
{
	Q_OBJECT
	
	/** The hiker data before user interaction starts. */
	const Hiker* init;
	
public:
	HikerDialog(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* init);
	~HikerDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Hiker* extractData();
	virtual bool changesMade() override;
	
private:
	void insertInitData();
	
private slots:
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewHikerDialogAndStore			(QWidget* parent, Database* db);
void			openEditHikerDialogAndStore			(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);
void			openDeleteHikersDialogAndExecute	(QWidget* parent, Database* db, QSet<BufferRowIndex> bufferRowIndices);



#endif // HIKER_DIALOG_H
