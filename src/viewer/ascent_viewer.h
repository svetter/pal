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

#ifndef ASCENT_VIEWER_H
#define ASCENT_VIEWER_H

#include "src/main/main_window.h"
#include "src/viewer/image_display.h"
#include "src/viewer/scalable_image_label.h"
#include "ui_ascent_viewer.h"



class AscentViewer : public QDialog, public Ui_AscentViewer {
	Q_OBJECT
	
	MainWindow* const mainWindow;
	Database* const db;
	const ItemTypesHandler* typesHandler;
	CompositeAscentsTable* const compAscents;
	CompositePeaksTable* const compPeaks;
	CompositeTripsTable* const compTrips;
	
	int currentViewRowIndex;
	ItemID currentAscentID;
	
	int firstAscentViewRowIndex;
	int previousAscentViewRowIndex;
	int nextAscentViewRowIndex;
	int lastAscentViewRowIndex;
	
	int firstAscentOfPeakViewRowIndex;
	int previousAscentOfPeakViewRowIndex;
	int nextAscentOfPeakViewRowIndex;
	int lastAscentOfPeakViewRowIndex;
	
	int currentAscentOfPeakIndex;
	int numAscentsOfPeak;
	
	QList<Photo> photos;
	int currentPhotoIndex;
	
	ScalableImageLabel* imageLabel;
	QImage image;
	
	bool descriptionEditable;
	bool photoDescriptionEditable;
	
	QMenu infoContextMenu;
	QAction* editAscentAction;
	QAction* editPeakAction;
	QAction* editTripAction;
	
public:
	AscentViewer(MainWindow* parent, Database* db, const ItemTypesHandler* typesHandler, int viewRowIndex);
	virtual ~AscentViewer();
	
private:
	// Initial setup
	void additionalUISetup();
	void connectUI();
	void setupContextMenus();
	void setupShortcuts();
	
	// Ascent change
	void changeToAscent(int viewRowIndex);
	void resetInfoLabels();
	void updateAscentInfo();
	void updateAscentNavigationTargets();
	void updateAscentNavigationButtonsEnabled();
	void updateAscentNavigationNumbers();
	void setupPhotos();
	
	// Photo change
	void changeToPhoto(int photoIndex);
	void updatePhoto();
	void updatePhotoIndexLabel();
	void updatePhotoButtonsEnabled();
	
	// Editing photos
	void moveCurrentPhoto(bool moveLeftNotRight);
	void addPhotos();
	void removeCurrentPhoto();
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
	
private:
	// Helpers
	void popupInfoContextMenu(QPoint pos);
	void handleChangesToUnderlyingData(int currentBufferRowIndex);
	
	// Exit behaviour
	void reject() override;
	void saveSplitterSizes();
	
	void restoreSplitterSizes();
};



#endif // ASCENT_VIEWER_H
