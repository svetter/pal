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
 * @file trip_dialog.h
 * 
 * This file declares the TripDialog class.
 */

#ifndef TRIP_DIALOG_H
#define TRIP_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/trip.h"
#include "ui_trip_dialog.h"



/**
 * Control class for the trip dialog.
 */
class TripDialog : public ItemDialog, public Ui_TripDialog
{
	Q_OBJECT
	
	/** The trip data before user interaction starts. */
	const Trip* init;
	
public:
	TripDialog(QWidget* parent, Database* db, DialogPurpose purpose, Trip* init);
	~TripDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Trip* extractData();
	virtual bool changesMade() override;
	
private:
	void insertInitData();
	
private slots:
	void handle_datesSpecifiedChanged();
	void handle_startDateChanged();
	void handle_endDateChanged();
	
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
};



BufferRowIndex	openNewTripDialogAndStore		(QWidget* parent, Database* db);
bool			openEditTripDialogAndStore		(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);
bool			openDeleteTripsDialogAndExecute	(QWidget* parent, Database* db, QSet<BufferRowIndex> bufferRowIndices);



#endif // TRIP_DIALOG_H
