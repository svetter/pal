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

#ifndef RANGE_DIALOG_H
#define RANGE_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/range.h"
#include "ui_range_dialog.h"



class RangeDialog : public ItemDialog, public Ui_RangeDialog
{
	Q_OBJECT
	
	const Range* init;
	
public:
	RangeDialog(QWidget* parent, Database* db, DialogPurpose purpose, Range* init);
	~RangeDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Range* extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewRangeDialogAndStore		(QWidget* parent, Database* db);
void			openEditRangeDialogAndStore		(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);
void			openDeleteRangeDialogAndExecute	(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);



#endif // RANGE_DIALOG_H
