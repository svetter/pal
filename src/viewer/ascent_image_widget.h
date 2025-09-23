/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file ascent_image_widget.h
 * 
 * This file declares the AscentImageWidget class.
 */

#ifndef ASCENT_IMAGE_WIDGET_H
#define ASCENT_IMAGE_WIDGET_H

#include "src/data/photo.h"
#include "src/db/database.h"
#include "src/viewer/scalable_image_label.h"
#include "ui_ascent_image_widget.h"

#include <QWidget>
#include <QTimer>



class AscentImageWidget : public QWidget, public Ui_AscentImageWidget
{
	Q_OBJECT
	
	/** The project's database. */
	Database* db;
	
	ItemID* currentAscentID;
	
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
	
	/** Indicates whether the photo description is currently set to be editable. */
	bool photoDescriptionEditable;
	
	/** Temporary global static variable for error messages printed when loading images. */
	inline static QString imageLoadErrorMessage = QString();
	
public:
	AscentImageWidget(QWidget* parent);
	virtual ~AscentImageWidget();
	
	// Initial setup
	void supplyPointers(Database* db, ItemID* currentAscentID);
private:
	void additionalUISetup();
	void connectUI();
	void setupShortcuts();
	void setupSlideshow();
	
public:
	// Ascent change
	void ascentAboutToChange();
	void ascentChanged();
private:
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
	
private slots:
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
	// Edit action
	void handle_photoDescriptionEditableChanged();
	
	// Files dropped on image frame
	void handle_filesDropped(QStringList filepaths);
	
	// Error message capture
	static void imageErrorMessageOccurred(const QString& message);
	
	// Exit behaviour
public:
	void aboutToExit();
private:
	void saveImplicitSettings();
public:
	void restoreImplicitSettings();
};



#endif // ASCENT_IMAGE_WIDGET_H
