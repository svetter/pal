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
 * @file range_dialog.h
 * 
 * This file declares the RangeDialog class.
 */

#ifndef RANGE_DIALOG_H
#define RANGE_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/range.h"
#include "ui_range_dialog.h"

using std::unique_ptr;



/**
 * Control class for the range dialog.
 */
class RangeDialog : public ItemDialog, public Ui_RangeDialog
{
	Q_OBJECT
	
	/** The range data before user interaction starts. */
	unique_ptr<const Range> init;
	
public:
	RangeDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Range> init);
	~RangeDialog();
	
	virtual QString getEditWindowTitle() override;
	
	unique_ptr<Range> extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewRangeDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db);
bool			openEditRangeDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex);
bool			openDeleteRangesDialogAndExecute	(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices);

BufferRowIndex	openRangeDialogAndStore				(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<Range> originalRange);



#endif // RANGE_DIALOG_H
