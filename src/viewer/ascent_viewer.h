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
 * @file ascent_viewer.h
 * 
 * This file declares the AscentViewer class.
 */

#ifndef ASCENT_VIEWER_H
#define ASCENT_VIEWER_H

#include "src/main/main_window.h"
#include "src/viewer/scalable_image_label.h"
#include "ui_ascent_viewer.h"



/**
 * Control class for the ascent viewer window.
 */
class AscentViewer : public QDialog, public Ui_AscentViewer {
	Q_OBJECT
	
	/** The application's main window. */
	MainWindow* const mainWindow;
	/** The project's database. */
	Database* const db;
	/** The application's item types handler. */
	const ItemTypesHandler* typesHandler;
	/** The composite ascents table. */
	CompositeAscentsTable* const compAscents;
	/** The composite peaks table. */
	CompositePeaksTable* const compPeaks;
/** The composite trips table. */
	CompositeTripsTable* const compTrips;
	
	/** The current view row index. */
	ViewRowIndex currentViewRowIndex;
	/** The ID of the currently displayed ascent, or an invalid ID if no ascent is displayed. */
	ItemID currentAscentID;

	/** The view row index of the first ascent in the composite ascents table. */
	ViewRowIndex    firstAscentViewRowIndex;
	/** The view row index of the ascent before the current one in the composite ascents table. */
	ViewRowIndex previousAscentViewRowIndex;
	/** The view row index of the ascent after the current one in the composite ascents table. */
	ViewRowIndex     nextAscentViewRowIndex;
	/** The view row index of the last ascent in the composite ascents table. */
	ViewRowIndex     lastAscentViewRowIndex;

	/** The view row index of the first ascent of the current ascent's peak in the composite ascents table. */
	ViewRowIndex    firstAscentOfPeakViewRowIndex;
	/** The view row index of the ascent before the current one of the current ascent's peak in the composite ascents table. */
	ViewRowIndex previousAscentOfPeakViewRowIndex;
	/** The view row index of the ascent after the current one of the current ascent's peak in the composite ascents table. */
	ViewRowIndex     nextAscentOfPeakViewRowIndex;
	/** The view row index of the last ascent of the current ascent's peak in the composite ascents table. */
	ViewRowIndex     lastAscentOfPeakViewRowIndex;
	
	/** Indicates that the current ascent is the n-th ascent of its peak. */
	int currentAscentOfPeakIndex;
	/** The number of ascents of the current ascent's peak. */
	int numAscentsOfPeak;
	
	/** List of all photos of the current ascent. */
	QList<Photo> photos;
	/** The index of the currently displayed photo in the photos list. -1 if no photo is displayed. */
	int currentPhotoIndex;
	
	/** The widget for displaying the image. */
	ScalableImageLabel* imageLabel;
	
	/** Indicates whether the ascent description is currently set to be editable. */
	bool descriptionEditable;
	/** Indicates whether the photo description is currently set to be editable. */
	bool photoDescriptionEditable;
	
	/** Context menu for the info area. */
	QMenu infoContextMenu;
	/** Context menu action for editing the current ascent. */
	QAction* editAscentAction;
	/** Context menu action for editing the current ascent's peak. */
	QAction* editPeakAction;
	/** Context menu action for editing the current ascent's trip. */
	QAction* editTripAction;
	
public:
	AscentViewer(MainWindow* parent, Database* db, const ItemTypesHandler* typesHandler, ViewRowIndex viewRowIndex);
	virtual ~AscentViewer();
	
private:
	// Initial setup
	void additionalUISetup();
	void connectUI();
	void setupContextMenus();
	void setupShortcuts();
	
	// Ascent change
	void changeToAscent(ViewRowIndex viewRowIndex);
	void resetInfoLabels();
	void updateInfoArea();
	void updateAscentNavigationTargets();
	void updateAscentNavigationButtonsEnabled();
	void updateAscentNavigationNumbers();
	void loadPhotosList();
	
	// Photo change
	void changeToPhoto(int photoIndex, bool saveDescriptionFirst = false);
	void updateImageFrameProperties(bool imagePresent, bool imageReadable);
	void updatePhotoIndexLabel();
	void updatePhotoButtonsEnabled();
	
	// Editing photos
	void moveCurrentPhoto(bool moveLeftNotRight);
	void addPhotosFromDialog();
	void addPhotos(QStringList filepaths);
	void removeCurrentPhoto();
	void replaceCurrentPhoto();
	void savePhotoDescription();
	void savePhotosList();
	
	// Editing description
	void saveDescription();
	
private slots:
	// Ascent navigation
	void handle_firstAscent();
	void handle_previousAscent();
	void handle_nextAscent();
	void handle_lastAscent();
	void handle_firstAscentOfPeak();
	void handle_previousAscentOfPeak();
	void handle_nextAscentOfPeak();
	void handle_lastAscentOfPeak();
	// Photo navigation
	void handle_firstPhoto();
	void handle_previousPhoto();
	void handle_nextPhoto();
	void handle_lastPhoto();
	// Changing photos
	void handle_movePhotoLeft();
	void handle_movePhotoRight();
	void handle_addPhotos();
	void handle_removePhoto();
	void handle_replacePhoto();
	void handle_relocatePhotos();
	// Right click
	void handle_rightClickOnAscentInfo(QPoint pos);
	void handle_rightClickOnPeakInfo(QPoint pos);
	void handle_rightClickOnTripInfo(QPoint pos);
	// Edit actions
	void handle_descriptionEditableChanged();
	void handle_photoDescriptionEditableChanged();
	void handle_editAscent();
	void handle_editPeak();
	void handle_editTrip();
	// Files dropped on image frame
	void handle_filesDropped(QStringList filepaths);
	
private:
	// Helpers
	void popupInfoContextMenu(QPoint pos);
	void handleChangesToUnderlyingData(BufferRowIndex currentBufferRowIndex);
	
	// Exit behaviour
	void reject() override;
	void saveSplitterSizes();
	
	void restoreSplitterSizes();
};



#endif // ASCENT_VIEWER_H
