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
	Database& db;
	/** The application's item types handler. */
	const ItemTypesHandler* typesHandler;
	/** The composite ascents table. */
	CompositeAscentsTable& compAscents;
	/** The composite peaks table. */
	CompositePeaksTable& compPeaks;
	/** The composite trips table. */
	CompositeTripsTable& compTrips;
	
	/** The current view row index. */
	ViewRowIndex currentViewRowIndex;
	/** The ID of the currently displayed ascent, or an invalid ID if no ascent is displayed. */
	ItemID currentAscentID;
	/** The ID of the currently displayed ascent's peak, or an invalid ID if no ascent is displayed or the current ascent has no peak. */
	ItemID currentPeakID;
	/** The ID of the currently displayed ascent's trip, or an invalid ID if no ascent is displayed or the current ascent has no trip. */
	ItemID currentTripID;

	/** The view row index of the first ascent in the composite ascents table. */
	ViewRowIndex    firstAscentViewRowIndex;
	/** The view row index of the ascent before the current one in the composite ascents table. */
	ViewRowIndex previousAscentViewRowIndex;
	/** The view row index of the ascent after the current one in the composite ascents table. */
	ViewRowIndex     nextAscentViewRowIndex;
	/** The view row index of the last ascent in the composite ascents table. */
	ViewRowIndex     lastAscentViewRowIndex;
	
	/** The number of ascents shown in the composite ascents table. */
	int numShownAscents;

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
	
	/** A timer used for the slideshow function. */
	QTimer slideshowTimer;
	/** Indicates whether the slideshow is currently running. */
	bool slideshowRunning;
	
	/** The widget for displaying the image. */
	ScalableImageLabel* imageLabel;
	
	/** Indicates whether the trip description is currently set to be editable. */
	bool tripDescriptionEditable;
	/** Indicates whether the ascent description is currently set to be editable. */
	bool ascentDescriptionEditable;
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
	
	/** Keyboard shortcut for going to a random ascent. */
	QShortcut* goToRandomAscentShortcut;
	
	/** Saved sizes for the description splitter before being collapsed for an ascent with no trip. */
	QList<int> descriptionSplitterSizes;
	
private:
	/** Temporary global static variable for error messages printed when loading images. */
	inline static QString imageLoadErrorMessage = QString();
	
public:
	AscentViewer(MainWindow* parent, Database& db, const ItemTypesHandler* typesHandler, ViewRowIndex viewRowIndex);
	virtual ~AscentViewer();
	
private:
	// Initial setup
	void additionalUISetup();
	void connectUI();
	void setupContextMenus();
	void setupShortcuts();
	void setupSlideshow();
	
	// Ascent change
	void changeToAscent(ViewRowIndex viewRowIndex);
	void resetInfoLabels();
	void updateInfoArea();
	void updateAscentNavigationTargets();
	void updateAscentNavigationButtonsEnabled();
	void updateAscentNavigationNumbers();
	void loadPhotosList();
	
	// Photo change
	void changeToPhoto(int photoIndex, bool saveDescriptionFirst);
	void updateImageFrameProperties(bool imagePresent, bool imageReadable);
	void updatePhotoIndexLabel();
	void updatePhotoButtonsEnabled();
	
	// Slideshow
	void startSlideshow(bool nextPhotoImmediately);
	void stopSlideshow();
	void restartSlideshowTimerIfRunning();
	
	// Editing photos
	void moveCurrentPhoto(bool moveLeftNotRight);
	void addPhotosFromDialog();
	void addPhotos(QStringList filepaths);
	void removeCurrentPhoto();
	void replaceCurrentPhoto();
	void savePhotoDescription();
	void savePhotosList();
	
	// Editing descriptions
	void saveTripDescription();
	void saveAscentDescription();
	
private slots:
	// Ascent navigation
	void handle_firstAscent();
	void handle_previousAscent();
	void handle_nextAscent();
	void handle_lastAscent();
	void handle_randomAscent();
	void handle_firstAscentOfPeak();
	void handle_previousAscentOfPeak();
	void handle_nextAscentOfPeak();
	void handle_lastAscentOfPeak();
	// Photo navigation
	void handle_firstPhoto();
	void handle_previousPhoto();
	void handle_nextPhoto();
	void handle_lastPhoto();
	// Slideshow
	void handle_toggleSlideshow();
	void handle_slideshowTimerTrigger();
	void handle_slideshowIntervalChanged();
	void handle_userInteractedWithImageLabel();
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
	void handle_tripDescriptionEditableChanged();
	void handle_ascentDescriptionEditableChanged();
	void handle_photoDescriptionEditableChanged();
	void handle_editAscent();
	void handle_editPeak();
	void handle_editTrip();
	// Files dropped on image frame
	void handle_filesDropped(QStringList filepaths);
	
	// Error message capture
	static void imageErrorMessageOccurred(const QString& message);
	
private:
	// Helpers
	void popupInfoContextMenu(QPoint pos);
	void handleChangesToUnderlyingData(BufferRowIndex currentBufferRowIndex);
	
	// Exit behaviour
	void reject() override;
	void saveImplicitSettings();
	void restoreImplicitSettings();
};



#endif // ASCENT_VIEWER_H
