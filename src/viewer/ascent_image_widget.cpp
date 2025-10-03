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
 * @file ascent_image_widget.cpp
 * 
 * This file defines the AscentImageWidget class.
 */

#include "ascent_image_widget.h"

#include "src/dialogs/ascent_dialog.h"
#include "src/tools/relocate_photos_dialog.h"
#include "src/settings/settings.h"

#include <QStyle>
#include <QImageReader>
#include <QColorSpace>



AscentImageWidget::AscentImageWidget(QWidget* parent) :
	QWidget(parent),
	db(nullptr),
	currentAscentID(nullptr),
	photos(QList<Photo>()),
	currentPhotoIndex(-1),
	slideshowTimer(QTimer(this)),
	slideshowRunning(false),
	imageLabel(nullptr),
	photoDescriptionEditable(false)
{
	setupUi(this);
	additionalUISetup();
	
	connectUI();
	
	setupShortcuts();
	setupSlideshow();
}

AscentImageWidget::~AscentImageWidget()
{
	delete imageLabel;
}



// INITIAL SETUP

void AscentImageWidget::supplyPointers(Database* db, const ItemID* currentAscentID)
{
	this->db				= db;
	this->currentAscentID	= currentAscentID;
}

void AscentImageWidget::additionalUISetup()
{
	imageLabel = new ScalableImageLabel(imageScrollArea);
	imageScrollArea->setBackgroundRole(QPalette::Dark);
	imageScrollArea->setWidget(imageLabel);
	imageScrollArea->setVisible(false);
	imageErrorGroupBox->setVisible(false);
	
	firstPhotoButton		->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastPhotoButton			->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	slideshowStartStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	addPhotosButton			->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	removePhotoButton		->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	
	movePhotoLeftButton		->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	movePhotoRightButton	->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
	
	photoDescriptionEditable	= editPhotoDescriptionButton->isChecked();
	handle_photoDescriptionEditableChanged();
	
	// Make QGroupBox titles turn gray when disabled (like on other widgets)
	QColor disabledColor = QApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
	QPalette disabledPalette = QPalette();
	disabledPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
	slideshowBox->setPalette(disabledPalette);
}

void AscentImageWidget::connectUI()
{
	// Photo navigation
	connect(firstPhotoButton,				&QToolButton::clicked,			this,	&AscentImageWidget::handle_firstPhoto);
	connect(previousPhotoButton,			&QToolButton::clicked,			this,	&AscentImageWidget::handle_previousPhoto);
	connect(nextPhotoButton,				&QToolButton::clicked,			this,	&AscentImageWidget::handle_nextPhoto);
	connect(lastPhotoButton,				&QToolButton::clicked,			this,	&AscentImageWidget::handle_lastPhoto);
	// Slideshow
	connect(slideshowStartStopButton,		&QToolButton::clicked,			this,	&AscentImageWidget::handle_toggleSlideshow);
	connect(slideshowIntervalSpinner,		&QSpinBox::valueChanged,		this,	&AscentImageWidget::handle_slideshowIntervalChanged);
	// Changing photos
	connect(movePhotoLeftButton,			&QToolButton::clicked,			this,	&AscentImageWidget::handle_movePhotoLeft);
	connect(movePhotoRightButton,			&QToolButton::clicked,			this,	&AscentImageWidget::handle_movePhotoRight);
	connect(addPhotosButton,				&QToolButton::clicked,			this,	&AscentImageWidget::handle_addPhotos);
	connect(removePhotoButton,				&QToolButton::clicked,			this,	&AscentImageWidget::handle_removePhoto);
	// Image file error box
	connect(imageErrorRemoveButton,			&QPushButton::clicked,			this,	&AscentImageWidget::handle_removePhoto);
	connect(imageErrorReplaceButton,		&QPushButton::clicked,			this,	&AscentImageWidget::handle_replacePhoto);
	connect(imageErrorRelocateButton,		&QPushButton::clicked,			this,	&AscentImageWidget::handle_relocatePhotos);
	// Edit button
	connect(editPhotoDescriptionButton,		&QToolButton::clicked,			this,	&AscentImageWidget::handle_photoDescriptionEditableChanged);
	// Drag and drop
	connect(imageFrame,						&FileDropFrame::filesDropped,	this,	&AscentImageWidget::handle_filesDropped);
}

/**
 * Creates keyboard shortcuts for UI buttons.
 */
void AscentImageWidget::setupShortcuts()
{
	firstPhotoButton			->setShortcut(QKeySequence(Qt::Key_Up));
	previousPhotoButton			->setShortcut(QKeySequence(Qt::Key_Left));
	nextPhotoButton				->setShortcut(QKeySequence(Qt::Key_Right));
	lastPhotoButton				->setShortcut(QKeySequence(Qt::Key_Down));
	
	movePhotoLeftButton			->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Left));
	movePhotoRightButton		->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Right));
}

/**
 * Prepares slideshow functionality.
 */
void AscentImageWidget::setupSlideshow()
{
	connect(&slideshowTimer, &QTimer::timeout, this, &AscentImageWidget::handle_slideshowTimerTrigger);
	connect(imageLabel, &ScalableImageLabel::userInteracted, this, &AscentImageWidget::handle_userInteractedWithImageLabel);
}



// ASCENT CHANGE

void AscentImageWidget::ascentAboutToChange()
{
	stopSlideshow();
	if (currentAscentID && currentAscentID->isValid()) {
		savePhotoDescription();
	}
}

void AscentImageWidget::ascentChanged()
{
	loadPhotosList();
	changeToPhoto(photos.isEmpty() ? -1 : 0, false);
	
	if (slideshowAutostartCheckbox->isChecked() && photos.size() > 1) {
		startSlideshow(false);
	}
}

/**
 * Loads the current state of the photo list for the current ascent from the database.
 * 
 * Does not change or reset currentPhotoIndex.
 */
void AscentImageWidget::loadPhotosList()
{
	photos.clear();
	
	QList<Photo> savedPhotos = db->photosTable.getPhotosForAscent(FORCE_VALID(*currentAscentID));
	if (!savedPhotos.isEmpty()) {
		photos = savedPhotos;
	}
}



// PHOTO CHANGE

/**
 * Navigates to the photo with the given index.
 * 
 * @param photoIndex			The index of the photo to load.
 * @param saveDescriptionFirst	Whether to save the description for the current photo before changing to the new one.
 */
void AscentImageWidget::changeToPhoto(int photoIndex, bool saveDescriptionFirst)
{
	if (saveDescriptionFirst) {
		savePhotoDescription();
	}
	
	currentPhotoIndex = photoIndex;
	
	if (currentPhotoIndex < 0 || photos.isEmpty()) {
		// Show empty image frame
		updateImageFrameProperties(false, false);
		// Hide and clear description widgets
		photoDescriptionLabel		->setVisible(false);
		photoDescriptionLineEdit	->setVisible(false);
		photoDescriptionLabel		->setText(QString());
		photoDescriptionLineEdit	->setText(QString());
		// Clear image tooltip
		imageLabel->setToolTip(QString());
	}
	else {
		QString filepath = photos.at(currentPhotoIndex).filepath;
		QImageReader::setAllocationLimit(512);
		
		// Set up error message capturing
		imageLoadErrorMessage.clear();
		qInstallMessageHandler([] (QtMsgType type, const QMessageLogContext& context, const QString& msg) {
			Q_UNUSED(type);
			Q_UNUSED(context);
			if (!msg.startsWith("QImageIOHandler: ")) return;
			QString trimmedMessage = msg;
			trimmedMessage.remove("QImageIOHandler: ");
			AscentImageWidget::imageErrorMessageOccurred(trimmedMessage);
		});
		
		QImageReader reader = QImageReader(filepath);
		reader.setAutoTransform(true);
		QImage image = reader.read();
		
		// Fetch printed error message
		qInstallMessageHandler(nullptr);
		QString extraErrorMessage = imageLoadErrorMessage;
		imageLoadErrorMessage.clear();
		if (!extraErrorMessage.isEmpty()) {
			extraErrorMessage = tr("\nMore details: %1.").arg(extraErrorMessage);
		}
		
		if (!image.isNull()) {	// Image loaded
			// Prepare image frame to show image
			updateImageFrameProperties(true, true);
			if (image.colorSpace().isValid()) image.convertToColorSpace(QColorSpace::SRgb);
			imageLabel->setImage(image);
		}
		else {	// Loading failed
			QString labelText = tr(
									"This image file cannot be shown:\n%1"
									"\nReason: %2."
									"%3"
									"\n\nYou can remove the image, replace the file, or mass relocate image files in the whole database.")
									.arg(filepath, reader.errorString(), extraErrorMessage);	// Error string is already translated
			imageErrorLabel->setText(labelText);
			
			// Show image error box
			updateImageFrameProperties(true, false);
		}
		
		// Fill (and show) description widgets
		photoDescriptionLabel		->setText(photos.at(currentPhotoIndex).description);
		photoDescriptionLineEdit	->setText(photos.at(currentPhotoIndex).description);
		photoDescriptionLabel		->setVisible(!photoDescriptionEditable);
		photoDescriptionLineEdit	->setVisible(photoDescriptionEditable);
		// Set image tooltip
		imageLabel->setToolTip(filepath);
	}
	
	updatePhotoIndexLabel();
	updatePhotoButtonsEnabled();
}

/**
 * Updates the image frame and its children to either show the frame empty (no images), or to show
 * the image scroll area or the image file error box.
 * 
 * @pre imageReadable implies imagePresent, i.e., imagePresent cannot be false while imageReadable is true.
 * 
 * @param imagePresent	Whether the current ascent has any images.
 * @param imageReadable	Whether the current image can be read and displayed.
 */
void AscentImageWidget::updateImageFrameProperties(bool imagePresent, bool imageReadable)
{
	assert(!imageReadable || imagePresent);
	
	if (!imagePresent) {
		// Empty image frame will be displayed
		imageFrame			->setFrameStyle(QFrame::StyledPanel);
		imageErrorGroupBox	->setVisible(false);
		imageScrollArea		->setVisible(false);
		imageLabel			->clearImage();
	}
	
	else if (!imageReadable) {
		// Image error box will be displayed
		imageScrollArea		->setVisible(false);
		imageErrorGroupBox	->setVisible(true);
		imageFrame			->setFrameStyle(QFrame::StyledPanel);
		imageFrame			->layout()->setContentsMargins(10, 10, 10, 10);
		imageLabel			->clearImage();
		return;
	}
	
	else {
		// Image will be displayed
		imageFrame			->setFrameStyle(QFrame::NoFrame);
		imageFrame			->layout()->setContentsMargins(0, 0, 0, 0);
		imageErrorGroupBox	->setVisible(false);
		imageScrollArea		->setVisible(true);
	}
}

/**
 * Updates the "n / N" style UI label next to the photo navigation buttons.
 */
void AscentImageWidget::updatePhotoIndexLabel()
{
	if (currentPhotoIndex < 0) {
		photoIndexLabel->setText(tr("No photos"));
	} else {
		photoIndexLabel->setText(tr("Photo %1 of %2").arg(currentPhotoIndex + 1).arg(photos.size()));
	}
}

/**
 * Updates the enabled state of the photo navigation, move and remove buttons.
 */
void AscentImageWidget::updatePhotoButtonsEnabled()
{
	firstPhotoButton			->setEnabled(currentPhotoIndex > 0);
	previousPhotoButton			->setEnabled(currentPhotoIndex > 0);
	nextPhotoButton				->setEnabled(currentPhotoIndex < photos.size() - 1);
	lastPhotoButton				->setEnabled(currentPhotoIndex < photos.size() - 1);
	
	slideshowBox				->setEnabled(photos.size() > 1);
	
	movePhotoLeftButton			->setEnabled(currentPhotoIndex > 0);
	movePhotoRightButton		->setEnabled(currentPhotoIndex < photos.size() - 1);
	
	editPhotoDescriptionButton	->setEnabled(!photos.isEmpty());
	removePhotoButton			->setEnabled(!photos.isEmpty());
}



// SLIDESHOW

/**
 * Starts the slideshow unless it is already running.
 * 
 * Disables editing of the photo description, changes the slideshow button icon and starts the timer.
 * 
 * @param nextPhotoImmediately	Whether to change to the next photo immediately, skipping the first waiting interval.
 */
void AscentImageWidget::startSlideshow(bool nextPhotoImmediately)
{
	if (slideshowRunning) return;
	
	if (photoDescriptionEditable) {
		editPhotoDescriptionButton->setChecked(false);
		handle_photoDescriptionEditableChanged();
	}
	
	if (isVisible()) {
		slideshowTimer.start(slideshowIntervalSpinner->value() * 1000);
	}
	slideshowStartStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	slideshowRunning = true;
	
	if (nextPhotoImmediately) handle_slideshowTimerTrigger();
}

/**
 * Stops the slideshow if it is running.
 * 
 * Changes the slideshow button icon and stops the timer.
 */
void AscentImageWidget::stopSlideshow()
{
	if (!slideshowRunning) return;
	
	slideshowTimer.stop();
	slideshowStartStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	slideshowRunning = false;
}

/**
 * Resets the slideshow timer if the slideshow is currently running, else does nothing.
 * 
 * To be called when the user mannually changes the photo.
 */
void AscentImageWidget::restartSlideshowTimerIfRunning()
{
	if (!slideshowRunning) return;
	slideshowTimer.start(slideshowIntervalSpinner->value() * 1000);
}



// EDITING PHOTOS

/**
 * Moves the current photo towards the beginning or end of the photo list.
 * 
 * @param moveLeftNotRight	Whether to move the photo towards the beginning (true) or end (false) of the list.
 */
void AscentImageWidget::moveCurrentPhoto(bool moveLeftNotRight)
{
	assert(photos.size() > 1);
	assert((moveLeftNotRight && currentPhotoIndex > 0) || (!moveLeftNotRight && currentPhotoIndex < photos.size() - 1));
	
	int newPhotoIndex = currentPhotoIndex + (moveLeftNotRight ? -1 : 1);
	photos.move(currentPhotoIndex, newPhotoIndex);
	currentPhotoIndex = newPhotoIndex;
	
	updatePhotoIndexLabel();
	updatePhotoButtonsEnabled();
	
	savePhotosList();
}

/**
 * Adds user-selected photos from a file dialog to the current ascent.
 * 
 * Phots are inserted after the current photo, or at the beginning of the list if it is empty.
 * The first added photo is then displayed.
 */
void AscentImageWidget::addPhotosFromDialog()
{
	QString preSelectedDir = QString();
	if (!photos.isEmpty()) {
		assert(currentPhotoIndex >= 0);
		QFileInfo(photos.at(currentPhotoIndex).filepath).path();
	}
	QStringList filepaths = openFileDialogForMultiPhotoSelection(*this, preSelectedDir);
	if (filepaths.isEmpty()) return;
	
	addPhotos(filepaths);
}

/**
 * Adds the given photos to the current ascent.
 * 
 * Phots are inserted after the current photo, or at the beginning of the list if it is empty.
 * The first added photo is then displayed.
 * 
 * @param filepaths	The filepaths of the photos to add.
 */
void AscentImageWidget::addPhotos(QStringList filepaths)
{
	if (filepaths.isEmpty()) return;
	
	savePhotoDescription();
	
	if (currentPhotoIndex < 0) currentPhotoIndex = -1;
	currentPhotoIndex++;	// Set to index of first inserted photo
	for (int i = 0; i < filepaths.size(); i++) {
		photos.insert(currentPhotoIndex + i, Photo(*currentAscentID, ItemID(), -1, filepaths.at(i), QString()));
	}
	savePhotosList();
	
	changeToPhoto(currentPhotoIndex, false);
}

/**
 * Removes the current photo from the current ascent and displays the next one, if present.
 */
void AscentImageWidget::removeCurrentPhoto()
{
	photos.removeAt(currentPhotoIndex);
	savePhotosList();
	
	int newPhotoIndex = std::min(currentPhotoIndex, (int) photos.size() - 1);
	changeToPhoto(newPhotoIndex, false);
	
	if (slideshowRunning && photos.size() < 2) stopSlideshow();
	restartSlideshowTimerIfRunning();
}

/**
 * Opens a file dialog and lets the user select a new file to replace the filepath for the current
 * photo, keeping the order and description.
 */
void AscentImageWidget::replaceCurrentPhoto()
{
	savePhotoDescription();
	
	/*: %1 is a filepath, so it is best if it remains at the end of the string. */
	QString dialogTitle = tr("Replace %1").arg(photos.at(currentPhotoIndex).filepath);
	QString preSelectedDir = QFileInfo(photos.at(currentPhotoIndex).filepath).path();
	QString filepath = openFileDialogForSinglePhotoSelection(*this, preSelectedDir, dialogTitle);
	if (filepath.isEmpty()) return;
	
	photos[currentPhotoIndex].filepath = filepath;
	savePhotosList();
	
	changeToPhoto(currentPhotoIndex, false);
}

/**
 * Saves the description for the current photo from the UI to the database.
 */
void AscentImageWidget::savePhotoDescription()
{
	if (currentPhotoIndex < 0 || photos.empty() || !photoDescriptionEditable) return;
	
	QString newDescription = photoDescriptionLineEdit->text();
	bool descriptionChanged = photos.at(currentPhotoIndex).description != newDescription;
	if (descriptionChanged) {
		photos[currentPhotoIndex].description = newDescription;
		savePhotosList();
	}
}

/**
 * Saves the current state of the photo list to the database.
 */
void AscentImageWidget::savePhotosList()
{
	for (int i = 0; i < photos.size(); i++) {
		photos[0].sortIndex = i;
	}
	db->beginChangingData();
	db->photosTable.updateRows(*this, FORCE_VALID(*currentAscentID), photos);
	db->finishChangingData();
}


// PHOTO NAVIGATION

/**
 * Event handler for the "Go to first photo" button.
 */
void AscentImageWidget::handle_firstPhoto()
{
	changeToPhoto(0, true);
	restartSlideshowTimerIfRunning();
}

/**
 * Event handler for the "Go to previous photo" button.
 */
void AscentImageWidget::handle_previousPhoto()
{
	changeToPhoto(currentPhotoIndex - 1, true);
	restartSlideshowTimerIfRunning();
}

/**
 * Event handler for the "Go to next photo" button.
 */
void AscentImageWidget::handle_nextPhoto()
{
	changeToPhoto(currentPhotoIndex + 1, true);
	restartSlideshowTimerIfRunning();
}

/**
 * Event handler for the "Go to last photo" button.
 */
void AscentImageWidget::handle_lastPhoto()
{
	changeToPhoto(photos.size() - 1, true);
	restartSlideshowTimerIfRunning();
}


// SLIDESHOW

/**
 * Starts the slideshow if it is not running and stops it otherwise.
 */
void AscentImageWidget::handle_toggleSlideshow()
{
	if (slideshowRunning) {
		stopSlideshow();
	} else {
		startSlideshow(true);
	}
}

/**
 * Trigger function for the slideshow timer, to be called when the set time has passed.
 */
void AscentImageWidget::handle_slideshowTimerTrigger()
{
	if (photos.size() < 2) return;
	int nextPhotoIndex = currentPhotoIndex + 1;
	if (nextPhotoIndex >= photos.size()) nextPhotoIndex = 0;
	changeToPhoto(nextPhotoIndex, true);
}

/**
 * Event handler for changes in the slideshow interval spin box.
 */
void AscentImageWidget::handle_slideshowIntervalChanged()
{
	slideshowTimer.setInterval(slideshowIntervalSpinner->value() * 1000);
}

/**
 * Event handler for user interaction with the image area.
 */
void AscentImageWidget::handle_userInteractedWithImageLabel()
{
	stopSlideshow();
}


// CHANGING PHOTOS

/**
 * Event handler for the "Move photo left" button.
 */
void AscentImageWidget::handle_movePhotoLeft()
{
	restartSlideshowTimerIfRunning();
	moveCurrentPhoto(true);
}

/**
 * Event handler for the "Move photo right" button.
 */
void AscentImageWidget::handle_movePhotoRight()
{
	restartSlideshowTimerIfRunning();
	moveCurrentPhoto(false);
}

/**
 * Event handler for the "Add photos" button.
 */
void AscentImageWidget::handle_addPhotos()
{
	stopSlideshow();
	addPhotosFromDialog();
}

/**
 * Event handler for the "Remove photo" buttons.
 */
void AscentImageWidget::handle_removePhoto()
{
	removeCurrentPhoto();
}

/**
 * Event handler for the "Replace" button in the file not found box.
 */
void AscentImageWidget::handle_replacePhoto()
{
	stopSlideshow();
	replaceCurrentPhoto();
}

/**
 * Event handler for the "Mass relocate" button in the file not found box.
 */
void AscentImageWidget::handle_relocatePhotos()
{
	savePhotoDescription();
	stopSlideshow();
	
	RelocatePhotosDialog* dialog = new RelocatePhotosDialog(*this, *db);
	
	auto callWhenClosed = [=]() {
		if (dialog->result() == QDialog::Accepted) {
			loadPhotosList();
			changeToPhoto(currentPhotoIndex, false);
		}
		delete dialog;
	};
	connect(dialog, &RelocatePhotosDialog::finished, callWhenClosed);
	
	dialog->open();
}


// EDIT ACTION

/**
 * Event handler for the button controlling photo description editability.
 */
void AscentImageWidget::handle_photoDescriptionEditableChanged()
{
	if (photoDescriptionEditable) {
		savePhotoDescription();
		photoDescriptionLabel->setText(photoDescriptionLineEdit->text());
	} else {
		photoDescriptionLineEdit->setText(photoDescriptionLabel->text());
		
		stopSlideshow();
	}
	
	photoDescriptionEditable = editPhotoDescriptionButton->isChecked();
	
	photoDescriptionLabel	->setVisible(!photoDescriptionEditable);
	photoDescriptionLineEdit->setVisible(photoDescriptionEditable);
}


// DRAG AND DROP

/**
 * Event handler for files dropped on the image frame.
 * 
 * Checks whether each file is a readable image, asks the user to confirm the addition of each one
 * if not, and adds the readable as well as the confirmed ones to the current ascent.
 * 
 * @param filepaths	The filepaths of the dropped files.
 */
void AscentImageWidget::handle_filesDropped(QStringList filepaths)
{
	QStringList checkedPaths = checkImageFilepathsAndAskUser(*this, filepaths);
	if (checkedPaths.isEmpty()) return;
	addPhotos(checkedPaths);
}



/**
 * Globally accessible static function for receiving error messages from QImageReader.
 * 
 * This function is used because qInstallMessageHandler() does not allow capturing in lambdas.
 * 
 * @param message	The error message with its prefix already removed.
 */
void AscentImageWidget::imageErrorMessageOccurred(const QString& message)
{
	imageLoadErrorMessage = message;
}



void AscentImageWidget::showEvent(QShowEvent* event)
{
	Q_UNUSED(event);
	if (slideshowRunning && !slideshowTimer.isActive()) {
		slideshowTimer.start();
	}
}

void AscentImageWidget::hideEvent(QHideEvent* event)
{
	Q_UNUSED(event);
	if (slideshowRunning) {
		slideshowTimer.stop();
	}
}



// EXIT BEHAVIOUR

/**
 * Prepares the ascent image widget for closing by saving data and implicit settings.
 */
void AscentImageWidget::aboutToExit()
{
	savePhotoDescription();
	saveImplicitSettings();
}

/**
 * Saves all implicit settings for the ascent image widget to settings.
 */
void AscentImageWidget::saveImplicitSettings()
{
	Settings::ascentViewer_slideshowInterval.set(slideshowIntervalSpinner->value());
	Settings::ascentViewer_slideshowAutostart.set(slideshowAutostartCheckbox->isChecked());
}

/**
 * Restores all implicit settings to the ascent image widget.
 */
void AscentImageWidget::restoreImplicitSettings()
{
	slideshowIntervalSpinner->setValue(Settings::ascentViewer_slideshowInterval.get());
	slideshowAutostartCheckbox->setChecked(Settings::ascentViewer_slideshowAutostart.get());
}
