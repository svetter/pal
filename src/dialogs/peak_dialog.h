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
 * @file peak_dialog.h
 * 
 * This file declares the PeakDialog class.
 */

#ifndef PEAK_DIALOG_H
#define PEAK_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/peak.h"
#include "ui_peak_dialog.h"



/**
 * Control class for the peak dialog.
 */
class PeakDialog : public ItemDialog, public Ui_PeakDialog
{
	Q_OBJECT
	
	/** The peak data before user interaction starts. */
	const Peak* init;
	
	/** The list of IDs corresponding to the regions selectable in the region combo box. */
	QList<ValidItemID> selectableRegionIDs;
	
public:
	PeakDialog(QWidget* parent, QMainWindow* mainWindow, Database& db, DialogPurpose purpose, Peak* init);
	~PeakDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Peak* extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_heightSpecifiedChanged();
	void handle_newRegion();
	
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewPeakDialogAndStore		(QWidget* parent, QMainWindow* mainWindow, Database& db);
BufferRowIndex	openDuplicatePeakDialogAndStore	(QWidget* parent, QMainWindow* mainWindow, Database& db, BufferRowIndex bufferRowIndex);
bool			openEditPeakDialogAndStore		(QWidget* parent, QMainWindow* mainWindow, Database& db, BufferRowIndex bufferRowIndex);
bool			openDeletePeaksDialogAndExecute	(QWidget* parent, QMainWindow* mainWindow, Database& db, QSet<BufferRowIndex> bufferRowIndices);

BufferRowIndex	openPeakDialogAndStore			(QWidget* parent, QMainWindow* mainWindow, Database& db, DialogPurpose purpose, Peak* originalPeak);



#endif // PEAK_DIALOG_H
