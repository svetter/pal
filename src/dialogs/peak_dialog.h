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

#ifndef PEAK_DIALOG_H
#define PEAK_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/peak.h"
#include "ui_peak_dialog.h"



class PeakDialog : public ItemDialog, public Ui_PeakDialog
{
	Q_OBJECT
	
	const Peak* init;
	
	QList<ValidItemID> selectableRegionIDs;
	
public:
	PeakDialog(QWidget* parent, Database* db, DialogPurpose purpose, Peak* init);
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
	
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



int openNewPeakDialogAndStore		(QWidget* parent, Database* db);
int openDuplicatePeakDialogAndStore	(QWidget* parent, Database* db, int bufferRowIndex);
void openEditPeakDialogAndStore		(QWidget* parent, Database* db, int bufferRowIndex);
void openDeletePeakDialogAndExecute	(QWidget* parent, Database* db, int bufferRowIndex);



#endif // PEAK_DIALOG_H
