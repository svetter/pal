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

#ifndef REGION_DIALOG_H
#define REGION_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/region.h"
#include "ui_region_dialog.h"



class RegionDialog : public ItemDialog, public Ui_RegionDialog
{
	Q_OBJECT
	
	const Region* init;
	
	QList<ValidItemID> selectableRangeIDs;
	QList<ValidItemID> selectableCountryIDs;
	
public:
	RegionDialog(QWidget* parent, Database* db, DialogPurpose purpose, Region* init);
	~RegionDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Region* extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_newRange();
	void handle_newCountry();
	
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewRegionDialogAndStore			(QWidget* parent, Database* db);
void			openEditRegionDialogAndStore		(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);
void			openDeleteRegionDialogAndExecute	(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);



#endif // REGION_DIALOG_H
