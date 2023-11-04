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
 * @file ascent_dialog.h
 * 
 * This file declares the AscentDialog class.
 */

#ifndef ASCENT_DIALOG_H
#define ASCENT_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/ascent.h"
#include "ui_ascent_dialog.h"

#include "list/hiker_list.h"
#include "list/photo_list.h"

#include <QWidget>



/**
 * Control class for the ascent dialog.
 */
class AscentDialog : public ItemDialog, public Ui_AscentDialog
{
	Q_OBJECT
	
	/** The ascent data before user interaction starts. */
	const Ascent* init;
	
	/** The list of IDs corresponding to the regions selectable in the region filter combo box. */
	QList<ValidItemID> selectableRegionIDs;
	/** The list of IDs corresponding to the peaks selectable in the peak combo box. */
	QList<ValidItemID> selectablePeakIDs;
	/** The list of IDs corresponding to the trips selectable in the trip combo box. */
	QList<ValidItemID> selectableTripIDs;
	
	/** The encapsulation for the hikers list. */
	HikersOnAscent hikersModel;
	/** The encapsulation for the photos list which enables drag and drop. */
	PhotosOfAscent photosModel;
	
public:
	AscentDialog(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* init);
	~AscentDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Ascent* extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_regionFilterChanged();
	void handle_newPeak();
	void handle_dateSpecifiedChanged();
	void handle_timeSpecifiedChanged();
	void handle_elevationGainSpecifiedChanged();
	void handle_difficultySystemChanged();
	void handle_newTrip();
	void handle_addHiker();
	void handle_removeHikers();
	void handle_addPhotos();
	void handle_removePhotos();
	void handle_photoSelectionChanged(const QItemSelection& selected = QItemSelection(), const QItemSelection& deselected = QItemSelection());
	
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
	void savePhotoDescriptionToList();
	void savePhotoDescriptionToList(const QItemSelection& selected, const QItemSelection& deselected);
};



BufferRowIndex	openNewAscentDialogAndStore			(QWidget* parent, Database* db);
BufferRowIndex	openDuplicateAscentDialogAndStore	(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);
void			openEditAscentDialogAndStore		(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);
void			openDeleteAscentDialogAndExecute	(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex);

QStringList openFileDialogForPhotosSelection(QWidget* parent);



#endif // ASCENT_DIALOG_H
