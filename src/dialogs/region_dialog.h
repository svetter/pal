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
 * @file region_dialog.h
 * 
 * This file declares the RegionDialog class.
 */

#ifndef REGION_DIALOG_H
#define REGION_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/region.h"
#include "ui_region_dialog.h"

using std::unique_ptr;



/**
 * Control class for the region dialog.
 */
class RegionDialog : public ItemDialog, public Ui_RegionDialog
{
	Q_OBJECT
	
	/** The region data before user interaction starts. */
	unique_ptr<const Region> init;
	
	/** The list of IDs corresponding to the ranges selectable in the range combo box. */
	QList<ValidItemID> selectableRangeIDs;
	/** The list of IDs corresponding to the countries selectable in the country combo box. */
	QList<ValidItemID> selectableCountryIDs;
	
public:
	RegionDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, const QString& windowTitle, unique_ptr<const Region> init);
	~RegionDialog();
	
	unique_ptr<Region> extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_newRange();
	void handle_newCountry();
	
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewRegionDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db);
bool			openEditRegionDialogAndStore		(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex);
bool			openDeleteRegionsDialogAndExecute	(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices);



#endif // REGION_DIALOG_H
