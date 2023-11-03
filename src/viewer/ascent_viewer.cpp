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
 * @file ascent_viewer.cpp
 * 
 * This file defines the AscentViewer class.
 */

#include "ascent_viewer.h"

#include <QDialog>
#include <QStyle>
#include <QImageReader>
#include <QMessageBox>
#include <QDir>
#include <QColorSpace>
#include <QFileDialog>



/**
 * Creates a new AscentViewer.
 * 
 * @param parent		The application's main window.
 * @param db			The project's database.
 * @param typesHandler	The application's ItemTypesHandler.
 * @param viewRowIndex	The view row index of the ascent to open in the viewer.
 */
AscentViewer::AscentViewer(MainWindow* parent, Database* db, const ItemTypesHandler* typesHandler, ViewRowIndex viewRowIndex) :
		QDialog(parent),
		mainWindow(parent),
		db(db),
		typesHandler(typesHandler),
		compAscents((CompositeAscentsTable*) typesHandler->get(ItemTypeAscent)->compTable),
		compPeaks((CompositePeaksTable*) typesHandler->get(ItemTypePeak)->compTable),
		compTrips((CompositeTripsTable*) typesHandler->get(ItemTypeTrip)->compTable),
		currentViewRowIndex(viewRowIndex),
		currentAscentID(ItemID()),
		photos(QList<Photo>()),
		descriptionEditable(false),
		photoDescriptionEditable(false),
		infoContextMenu(QMenu(this))
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/ascent_viewer_multisize_square.ico"));
	additionalUISetup();
	
	connectUI();
	setupContextMenus();
	setupShortcuts();
	
	changeToAscent(viewRowIndex);
}

/**
 * Destroys the AscentViewer.
 */
AscentViewer::~AscentViewer()
{
	delete imageLabel;
}



// INITIAL SETUP

/**
 * Configures stretch, restores geometry, sets button icons, and more UI setup.
 */
void AscentViewer::additionalUISetup()
{
	centralSplitter->setStretchFactor(0, 1);
	centralSplitter->setStretchFactor(1, 2);
	centralSplitter->setSizes({ centralSplitter->size().width() / 2, centralSplitter->size().width() / 2 });
	
	if (Settings::rememberWindowPositions.get()) {
		restoreDialogGeometry(this, mainWindow, &Settings::ascentViewer_geometry);
		restoreSplitterSizes();
	}
	
	
	peakVolcanoCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
	ascentTraverseCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
	
	imageLabel = new ScalableImageLabel(imageScrollArea);
	imageScrollArea->setBackgroundRole(QPalette::Dark);
	imageScrollArea->setWidget(imageLabel);
	
	firstPhotoButton		->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastPhotoButton			->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	addPhotosButton			->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	removePhotoButton		->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	
	firstAscentButton		->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastAscentButton		->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	firstAscentOfPeakButton	->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastAscentOfPeakButton	->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	
	movePhotoLeftButton		->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	movePhotoRightButton	->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
	
	descriptionEditable = editDescriptionButton->isChecked();
	photoDescriptionEditable = editPhotoDescriptionButton->isChecked();
	handle_descriptionEditableChanged();
	handle_photoDescriptionEditableChanged();
}

/**
 * Connects interactive UI elements to event handler functions.
 */
void AscentViewer::connectUI()
{
	// Ascent navigation
	connect(firstAscentButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_firstAscent);
	connect(previousAscentButton,		&QToolButton::clicked,	this,	&AscentViewer::handle_previousAscent);
	connect(nextAscentButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_nextAscent);
	connect(lastAscentButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_lastAscent);
	connect(firstAscentOfPeakButton,	&QToolButton::clicked,	this,	&AscentViewer::handle_firstAscentOfPeak);
	connect(previousAscentOfPeakButton,	&QToolButton::clicked,	this,	&AscentViewer::handle_previousAscentOfPeak);
	connect(nextAscentOfPeakButton,		&QToolButton::clicked,	this,	&AscentViewer::handle_nextAscentOfPeak);
	connect(lastAscentOfPeakButton,		&QToolButton::clicked,	this,	&AscentViewer::handle_lastAscentOfPeak);
	// Photo navigation
	connect(firstPhotoButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_firstPhoto);
	connect(previousPhotoButton,		&QToolButton::clicked,	this,	&AscentViewer::handle_previousPhoto);
	connect(nextPhotoButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_nextPhoto);
	connect(lastPhotoButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_lastPhoto);
	// Changing photos
	connect(movePhotoLeftButton,		&QToolButton::clicked,	this,	&AscentViewer::handle_movePhotoLeft);
	connect(movePhotoRightButton,		&QToolButton::clicked,	this,	&AscentViewer::handle_movePhotoRight);
	connect(addPhotosButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_addPhotos);
	connect(removePhotoButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_removePhoto);
	// Edit buttons
	connect(editDescriptionButton,		&QToolButton::clicked,	this,	&AscentViewer::handle_descriptionEditableChanged);
	connect(editPhotoDescriptionButton,	&QToolButton::clicked,	this,	&AscentViewer::handle_photoDescriptionEditableChanged);
	// Context menus
	connect(tripInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnTripInfo);
	connect(peakInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnPeakInfo);
	connect(ascentInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnAscentInfo);
}

/**
 * Populates the info area context menu.
 */
void AscentViewer::setupContextMenus()
{
	editAscentAction	= infoContextMenu.addAction(tr("Edit ascent..."),	this,	&AscentViewer::handle_editAscent);
	editPeakAction		= infoContextMenu.addAction(tr("Edit peak..."),		this,	&AscentViewer::handle_editPeak);
	editTripAction		= infoContextMenu.addAction(tr("Edit trip..."),		this,	&AscentViewer::handle_editTrip);
	
	editAscentAction	->setIcon(QIcon(":/icons/ascent.svg"));
	editPeakAction		->setIcon(QIcon(":/icons/peak.svg"));
	editTripAction		->setIcon(QIcon(":/icons/trip.svg"));
}

/**
* Creates keyboard shortcuts for UI buttons.
*/
void AscentViewer::setupShortcuts()
{
	firstAscentButton			->setShortcut(QKeySequence(Qt::Key_8));
	previousAscentButton		->setShortcut(QKeySequence(Qt::Key_4));
	nextAscentButton			->setShortcut(QKeySequence(Qt::Key_6));
	lastAscentButton			->setShortcut(QKeySequence(Qt::Key_2));
	
	firstAscentOfPeakButton		->setShortcut(QKeySequence(Qt::ALT | Qt::Key_8));
	previousAscentOfPeakButton	->setShortcut(QKeySequence(Qt::ALT | Qt::Key_4));
	nextAscentOfPeakButton		->setShortcut(QKeySequence(Qt::ALT | Qt::Key_6));
	lastAscentOfPeakButton		->setShortcut(QKeySequence(Qt::ALT | Qt::Key_2));
	
	firstPhotoButton			->setShortcut(QKeySequence(Qt::Key_Up));
	previousPhotoButton			->setShortcut(QKeySequence(Qt::Key_Left));
	nextPhotoButton				->setShortcut(QKeySequence(Qt::Key_Right));
	lastPhotoButton				->setShortcut(QKeySequence(Qt::Key_Down));
	
	movePhotoLeftButton			->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Left));
	movePhotoRightButton		->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Right));
}



// ASCENT CHANGE

/**
 * Navigates to the ascent with the given view row index.
 * 
 * @param viewRowIndex	The view row index of the ascent to load.
 */
void AscentViewer::changeToAscent(ViewRowIndex viewRowIndex)
{
	saveDescription();
	savePhotoDescription();
	
	currentViewRowIndex	= viewRowIndex;
	BufferRowIndex bufferRowIndex = compAscents->getBufferRowIndexForViewRow(viewRowIndex);
	currentAscentID = db->ascentsTable->getPrimaryKeyAt(bufferRowIndex);
	// Update main window selection
	mainWindow->updateSelectionAfterUserAction(*typesHandler->get(ItemTypeAscent), currentViewRowIndex);
	
	updateInfoArea();
	setupPhotos();
	updateAscentNavigationTargets();
	updateAscentNavigationButtonsEnabled();
	updateAscentNavigationNumbers();
}

/**
 * Clears all ascent-specific contents from the info area and leaves the labels empty or hidden.
 */
void AscentViewer::resetInfoLabels()
{
	tripNameLabel				->setText		(QString());
	tripDatesLabel				->setText		(QString());
	
	peakNameLabel				->setText		(QString());
	peakHeightLabel				->setText		(QString());
	peakVolcanoCheckbox			->setChecked	(false);
	peakRegionLabel				->setText		(QString());
	peakRangeLabel				->setText		(QString());
	peakCountryLabel			->setText		(QString());
	peakContinentLabel			->setText		(QString());
	peakMapsLinkLabel			->setText		(QString());
	peakEarthLinkLabel			->setText		(QString());
	peakWikipediaLinkLabel		->setText		(QString());
	
	ascentTitleLabel			->setText		(QString());
	ascentDateLabel				->setText		(QString());
	ascentTimeLabel				->setText		(QString());
	ascentPeakOnDayLabel		->setText		(QString());
	ascentElevationGainLabel	->setText		(QString());
	ascentHikeKindLabel			->setText		(QString());
	ascentTraverseCheckbox		->setChecked	(false);
	ascentDifficultyLabel		->setText		(QString());
	ascentParticipantsLabel		->setText		(QString());
	
	peakLinksBox				->setVisible	(false);
	peakMapsLinkLabel			->setVisible	(false);
	peakEarthLinkLabel			->setVisible	(false);
	peakWikipediaLinkLabel		->setVisible	(false);
	ascentTitleLabel			->setVisible	(false);
	ascentInfoLine				->setVisible	(false);
	ascentParticipantsBox		->setVisible	(false);
}

/**
 * Collects information about the current ascent, its peak and trip and displays it in the info
 * area.
 */
void AscentViewer::updateInfoArea()
{
	resetInfoLabels();
	
	BufferRowIndex ascentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	
	ItemID tripID = db->ascentsTable->tripIDColumn->getValueAt(ascentBufferRowIndex);
	if (tripID.isValid()) {
		BufferRowIndex tripBufferRowIndex = db->tripsTable->getBufferIndexForPrimaryKey(tripID.forceValid());
		tripNameLabel			->setText	(compAscents->tripColumn			->getFormattedValueAt(ascentBufferRowIndex).toString());
		QString startDate					= compTrips->startDateColumn		->getFormattedValueAt(tripBufferRowIndex).toString();
		QString endDate						= compTrips->endDateColumn			->getFormattedValueAt(tripBufferRowIndex).toString();
		QString dateRange = startDate;
		if (startDate != endDate) {
			QString length = compTrips->lengthColumn->getFormattedValueAt(tripBufferRowIndex).toString();
			dateRange = startDate + " – " + endDate + " (" + length + (")");
		}
		tripDatesLabel			->setText	(dateRange);
	}
	
	ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(ascentBufferRowIndex);
	if (peakID.isValid()) {
		BufferRowIndex peakBufferRowIndex = db->peaksTable->getBufferIndexForPrimaryKey(peakID.forceValid());
		peakNameLabel			->setText	(compAscents->peakColumn			->getFormattedValueAt(ascentBufferRowIndex).toString());
		peakHeightLabel			->setText	(compAscents->peakHeightColumn		->getFormattedValueAt(ascentBufferRowIndex).toString());
		peakVolcanoCheckbox		->setChecked(compAscents->volcanoColumn			->getRawValueAt(ascentBufferRowIndex).toBool());
		peakRegionLabel			->setText	(compAscents->regionColumn			->getFormattedValueAt(ascentBufferRowIndex).toString());
		peakRangeLabel			->setText	(compAscents->rangeColumn			->getFormattedValueAt(ascentBufferRowIndex).toString());
		peakCountryLabel		->setText	(compAscents->countryColumn			->getFormattedValueAt(ascentBufferRowIndex).toString());
		peakContinentLabel		->setText	(compAscents->continentColumn		->getFormattedValueAt(ascentBufferRowIndex).toString());
		QString mapsLink					= db->peaksTable->mapsLinkColumn	->getValueAt(peakBufferRowIndex).toString();
		QString earthLink					= db->peaksTable->earthLinkColumn	->getValueAt(peakBufferRowIndex).toString();
		QString wikiLink					= db->peaksTable->wikiLinkColumn	->getValueAt(peakBufferRowIndex).toString();
		if (!mapsLink.isEmpty() || !earthLink.isEmpty() || !wikiLink.isEmpty()) peakLinksBox->setVisible(true);
		if (!mapsLink.isEmpty()) {
			peakMapsLinkLabel		->setText("[" + tr("Google Maps") + "](" + mapsLink + ")");
			peakMapsLinkLabel		->setVisible(true);
		}
		if (!earthLink.isEmpty()) {
			peakEarthLinkLabel		->setText("[" + tr("Google Earth") + "](" + earthLink + ")");
			peakEarthLinkLabel		->setVisible(true);
		}
		if (!wikiLink.isEmpty()) {
			peakWikipediaLinkLabel	->setText("[" + tr("Wikipedia") + "](" + wikiLink + ")");
			peakWikipediaLinkLabel	->setVisible(true);
		}
	}
	
	QString ascentTitle						= compAscents->titleColumn			->getFormattedValueAt(ascentBufferRowIndex).toString();
	if (!ascentTitle.isEmpty()) {
		ascentTitleLabel		->setText	(compAscents->titleColumn			->getFormattedValueAt(ascentBufferRowIndex).toString());
		ascentTitleLabel		->setVisible(true);
		ascentInfoLine			->setVisible(true);
	}
	ascentDateLabel				->setText	(compAscents->dateColumn			->getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentTimeLabel				->setText	(db->ascentsTable->timeColumn		->getValueAt			(ascentBufferRowIndex).toString());
	ascentPeakOnDayLabel		->setText	(db->ascentsTable->peakOnDayColumn	->getValueAt			(ascentBufferRowIndex).toString() + ".");
	ascentElevationGainLabel	->setText	(compAscents->elevationGainColumn	->getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentHikeKindLabel			->setText	(compAscents->hikeKindColumn		->getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentTraverseCheckbox		->setChecked(compAscents->traverseColumn		->getRawValueAt			(ascentBufferRowIndex).toBool());
	ascentDifficultyLabel		->setText	(compAscents->difficultyColumn		->getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentPeakOrdinalLabel		->setText	(compAscents->peakOrdinalColumn		->getFormattedValueAt	(ascentBufferRowIndex).toString());
	
	QString hikersList = compAscents->hikersColumn->getFormattedValueAt(ascentBufferRowIndex).toString();
	if (!hikersList.isEmpty()) {
		ascentParticipantsLabel	->setText	(hikersList);
		ascentParticipantsBox->setVisible(true);
	}
	
	descriptionTextBrowser->setText(db->ascentsTable->descriptionColumn->getValueAt(ascentBufferRowIndex).toString());
}

/**
 * Updates view row indices which serve as jump targets for the ascent navigation buttons.
 * 
 * Invalid jump targets (disabled buttons) are set to -1, e.g., when the current ascent is the
 * first one in the list as currently sorted and filtered, which will disable the 'first ascent'
 * and 'previous ascent' buttons.
 * 
 * For ascents of the same peak, the sorting and filtering of the ascents table apply as well.
 * 
 * @see updateAscentNavigationButtonsEnabled()
 * @see updateAscentNavigationNumbers()
 */
void AscentViewer::updateAscentNavigationTargets()
{
	ViewRowIndex minViewRowIndex	= ViewRowIndex(0);
	ViewRowIndex maxViewRowIndex	= ViewRowIndex(compAscents->rowCount() - 1);
	
	firstAscentViewRowIndex		= (currentViewRowIndex == minViewRowIndex) ? ViewRowIndex() : minViewRowIndex;
	previousAscentViewRowIndex	= (currentViewRowIndex == minViewRowIndex) ? ViewRowIndex() : (currentViewRowIndex - 1);
	nextAscentViewRowIndex		= (currentViewRowIndex == maxViewRowIndex) ? ViewRowIndex() : (currentViewRowIndex + 1);
	lastAscentViewRowIndex		= (currentViewRowIndex == maxViewRowIndex) ? ViewRowIndex() : maxViewRowIndex;
	
	firstAscentOfPeakViewRowIndex		= ViewRowIndex();
	previousAscentOfPeakViewRowIndex	= ViewRowIndex();
	nextAscentOfPeakViewRowIndex		= ViewRowIndex();
	lastAscentOfPeakViewRowIndex		= ViewRowIndex();
	
	currentAscentOfPeakIndex	= -1;
	numAscentsOfPeak			= -1;
	
	BufferRowIndex bufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(bufferRowIndex);
	if (peakID.isValid()) {
		QList<BufferRowIndex> matchingBufferRowIndices = db->ascentsTable->getMatchingBufferRowIndices(db->ascentsTable->peakIDColumn, peakID.asQVariant());
		// Find matching view row indices (some or all ascents of the same peak may be filtered out)
		QList<ViewRowIndex> ascentOfPeakViewRowIndices = QList<ViewRowIndex>();
		for (const BufferRowIndex& matchingBufferRowIndex : matchingBufferRowIndices) {
			ViewRowIndex matchingViewRowIndex = compAscents->findViewRowIndexForBufferRow(matchingBufferRowIndex);
			if (matchingViewRowIndex.isInvalid()) continue;
			ascentOfPeakViewRowIndices.append(matchingViewRowIndex);
		}
		assert(!ascentOfPeakViewRowIndices.isEmpty());	// The current ascent has to be in the list
		
		ViewRowIndex minAscentOfPeakViewRowIndex	= ascentOfPeakViewRowIndices.first();
		ViewRowIndex maxAscentOfPeakViewRowIndex	= ascentOfPeakViewRowIndices.last();
		
		if (currentViewRowIndex > minAscentOfPeakViewRowIndex) {
			firstAscentOfPeakViewRowIndex		= minAscentOfPeakViewRowIndex;
			previousAscentOfPeakViewRowIndex	= ascentOfPeakViewRowIndices.at(ascentOfPeakViewRowIndices.indexOf(currentViewRowIndex) - 1);
		}
		if (currentViewRowIndex < maxAscentOfPeakViewRowIndex) {
			nextAscentOfPeakViewRowIndex		= ascentOfPeakViewRowIndices.at(ascentOfPeakViewRowIndices.indexOf(currentViewRowIndex) + 1);
			lastAscentOfPeakViewRowIndex		= maxAscentOfPeakViewRowIndex;
		}
		
		currentAscentOfPeakIndex	= ascentOfPeakViewRowIndices.indexOf(currentViewRowIndex);
		numAscentsOfPeak			= ascentOfPeakViewRowIndices.size();
	}
}

/**
 * Updates the enabled state of the ascent navigation buttons.
 * 
 * @pre updateAscentNavigationTargets() should be called first.
 */
void AscentViewer::updateAscentNavigationButtonsEnabled()
{
	firstAscentButton			->setEnabled(firstAscentViewRowIndex			.isValid());
	previousAscentButton		->setEnabled(previousAscentViewRowIndex			.isValid());
	nextAscentButton			->setEnabled(nextAscentViewRowIndex				.isValid());
	lastAscentButton			->setEnabled(lastAscentViewRowIndex				.isValid());
	
	firstAscentOfPeakButton		->setEnabled(firstAscentOfPeakViewRowIndex		.isValid());
	previousAscentOfPeakButton	->setEnabled(previousAscentOfPeakViewRowIndex	.isValid());
	nextAscentOfPeakButton		->setEnabled(nextAscentOfPeakViewRowIndex		.isValid());
	lastAscentOfPeakButton		->setEnabled(lastAscentOfPeakViewRowIndex		.isValid());
}

/**
 * Updates the "n / N" style UI labels next to the ascent navigation buttons.
 * 
 * @pre updateAscentNavigationTargets() should be called first.
 */
void AscentViewer::updateAscentNavigationNumbers()
{
	QString allAscentsNewText = QString::number(currentViewRowIndex.get() + 1) + " / " + QString::number(lastAscentViewRowIndex.get() + 1);
	allAscentsNumberLabel->setText(allAscentsNewText);
	allAscentsNumberLabel->setEnabled(lastAscentViewRowIndex.get() > 0);
	
	QString peakAscentsNewText = QString::number(currentAscentOfPeakIndex + 1) + " / " + QString::number(numAscentsOfPeak);
	ascentOfPeakNumberLabel->setText(peakAscentsNewText);
	ascentOfPeakNumberLabel->setEnabled(numAscentsOfPeak > 1);
}

/**
 * Updates the current photo list for the current ascent and displays the first one, if present.
 */
void AscentViewer::setupPhotos()
{
	photos.clear();
	int newPhotoIndex = -1;
	
	QList<Photo> savedPhotos = db->photosTable->getPhotosForAscent(currentAscentID.forceValid());
	if (!savedPhotos.isEmpty()) {
		photos = savedPhotos;
		newPhotoIndex = 0;
	}
	
	changeToPhoto(newPhotoIndex);
}



/**
 * Navigates to the photo with the given index.
 * 
 * @param photoIndex			The index of the photo to load.
 * @param saveDescriptionFirst	Whether to save the description for the current photo before changing to the new one.
 */
void AscentViewer::changeToPhoto(int photoIndex, bool saveDescriptionFirst)
{
	if (saveDescriptionFirst) {
		savePhotoDescription();
	}
	
	currentPhotoIndex = photoIndex;
	
	photoDescriptionLabel	->setText(QString());
	photoDescriptionLineEdit->setText(QString());
	photoDescriptionLabel	->setVisible(false);
	photoDescriptionLineEdit->setVisible(false);
	imageLabel				->setToolTip(QString());
	
	if (currentPhotoIndex < 0 || photos.isEmpty()) {
		imageLabel->clearImage();
		return;
	}
	
	QString filepath = photos.at(currentPhotoIndex).filepath;
	QImageReader reader(filepath);
	reader.setAutoTransform(true);
	const QImage newImage = reader.read();
	if (newImage.isNull()) {
		qDebug() << "Error reading" << filepath << reader.errorString();
		imageLabel->clearImage();
		
		QString title = tr("File error");
		QString message = tr("Photo could not be loaded:")
				+ "\n" + filepath
				+ "\n\n" + tr("Do you want to remove it from this ascent?");
		QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		QMessageBox::StandardButton result = QMessageBox::warning(this, title, message, buttons);
		
		if (result == QMessageBox::Yes) {
			removeCurrentPhoto();	// calls changeToPhoto() back, recursing until valid image is found or all photos removed
		}
	}
	else {
		image = newImage;
		if (image.colorSpace().isValid()) image.convertToColorSpace(QColorSpace::SRgb);
		imageLabel->setImage(image);
	}
	
	photoDescriptionLabel	->setText(photos.at(currentPhotoIndex).description);
	photoDescriptionLineEdit->setText(photos.at(currentPhotoIndex).description);
	photoDescriptionLabel	->setVisible(!photoDescriptionEditable);
	photoDescriptionLineEdit->setVisible(photoDescriptionEditable);
	imageLabel				->setToolTip(filepath);
	
	updatePhotoIndexLabel();
	updatePhotoButtonsEnabled();
}

/**
 * Updates the "n / N" style UI label next to the photo navigation buttons.
 */
void AscentViewer::updatePhotoIndexLabel()
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
void AscentViewer::updatePhotoButtonsEnabled()
{
	firstPhotoButton		->setEnabled(currentPhotoIndex > 0);
	previousPhotoButton		->setEnabled(currentPhotoIndex > 0);
	nextPhotoButton			->setEnabled(currentPhotoIndex < photos.size() - 1);
	lastPhotoButton			->setEnabled(currentPhotoIndex < photos.size() - 1);
	
	movePhotoLeftButton		->setEnabled(currentPhotoIndex > 0);
	movePhotoRightButton	->setEnabled(currentPhotoIndex < photos.size() - 1);
	
	removePhotoButton		->setEnabled(!photos.isEmpty());
}



// EDITING PHOTOS

/**
 * Moves the current photo towards the beginning or end of the photo list.
 * 
 * @param moveLeftNotRight	Whether to move the photo towards the beginning (true) or end (false) of the list.
 */
void AscentViewer::moveCurrentPhoto(bool moveLeftNotRight)
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
void AscentViewer::addPhotos()
{
	QStringList filepaths = openFileDialogForPhotosSelection(this);
	if (filepaths.isEmpty()) return;
	
	savePhotoDescription();
	
	if (currentPhotoIndex < 0) currentPhotoIndex = -1;
	currentPhotoIndex++;	// Set to index of first inserted photo
	for (int i = 0; i < filepaths.size(); i++) {
		photos.insert(currentPhotoIndex + i, Photo(currentAscentID, ItemID(), -1, filepaths.at(i), QString()));
	}
	savePhotosList();
	
	changeToPhoto(currentPhotoIndex);
}

/**
 * Removes the current photo from the current ascent and displays the next one, if present.
 */
void AscentViewer::removeCurrentPhoto()
{
	photos.removeAt(currentPhotoIndex);
	savePhotosList();
	
	int newPhotoIndex = std::min(currentPhotoIndex, (int) photos.size() - 1);
	changeToPhoto(newPhotoIndex);
}

/**
 * Saves the description for the current photo from the UI to the database.
 */
void AscentViewer::savePhotoDescription()
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
void AscentViewer::savePhotosList()
{
	for (int i = 0; i < photos.size(); i++) {
		photos[0].sortIndex = i;
	}
	db->photosTable->updateRows(this, currentAscentID.forceValid(), photos);
}



// EDITING DESCRIPTION

/**
 * Saves the description for the current ascent from the UI to the database.
 */
void AscentViewer::saveDescription()
{
	if (currentAscentID.isInvalid() || !descriptionEditable) return;
	
	QString newDescription = descriptionTextBrowser->toPlainText();
	bool descriptionChanged = db->ascentsTable->descriptionColumn->getValueFor(currentAscentID.forceValid()) != newDescription;
	if (descriptionChanged) {
		db->ascentsTable->updateCell(this, currentAscentID.forceValid(), db->ascentsTable->descriptionColumn, newDescription);
	}
}



// === UI EVENT HANDLERS ===

// ASCENT NAVIGATION

/**
 * Event handler for the "Go to first ascent" button.
 */
void AscentViewer::handle_firstAscent()
{
	changeToAscent(firstAscentViewRowIndex);
}

/**
 * Event handler for the "Go to previous ascent" button.
 */
void AscentViewer::handle_previousAscent()
{
	changeToAscent(previousAscentViewRowIndex);
}

/**
 * Event handler for the "Go to next ascent" button.
 */
void AscentViewer::handle_nextAscent()
{
	changeToAscent(nextAscentViewRowIndex);
}

/**
 * Event handler for the "Go to last ascent" button.
 */
void AscentViewer::handle_lastAscent()
{
	changeToAscent(lastAscentViewRowIndex);
}

/**
 * Event handler for the "Go to first ascent of same peak" button.
 */
void AscentViewer::handle_firstAscentOfPeak()
{
	changeToAscent(firstAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to previous ascent of same peak" button.
 */
void AscentViewer::handle_previousAscentOfPeak()
{
	changeToAscent(previousAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to next ascent of same peak" button.
 */
void AscentViewer::handle_nextAscentOfPeak()
{
	changeToAscent(nextAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to last ascent of same peak" button.
 */
void AscentViewer::handle_lastAscentOfPeak()
{
	changeToAscent(lastAscentOfPeakViewRowIndex);
}


// PHOTO NAVIGATION

/**
 * Event handler for the "Go to first photo" button.
 */
void AscentViewer::handle_firstPhoto()
{
	changeToPhoto(0, true);
}

/**
 * Event handler for the "Go to previous photo" button.
 */
void AscentViewer::handle_previousPhoto()
{
	changeToPhoto(currentPhotoIndex - 1, true);
}

/**
 * Event handler for the "Go to next photo" button.
 */
void AscentViewer::handle_nextPhoto()
{
	changeToPhoto(currentPhotoIndex + 1, true);
}

/**
 * Event handler for the "Go to last photo" button.
 */
void AscentViewer::handle_lastPhoto()
{
	changeToPhoto(photos.size() - 1, true);
}


// CHANGING PHOTOS

/**
 * Event handler for the "Move photo left" button.
 */
void AscentViewer::handle_movePhotoLeft()
{
	moveCurrentPhoto(true);
}

/**
 * Event handler for the "Move photo right" button.
 */
void AscentViewer::handle_movePhotoRight()
{
	moveCurrentPhoto(false);
}

/**
 * Event handler for the "Add photos" button.
 */
void AscentViewer::handle_addPhotos()
{
	addPhotos();
}

/**
 * Event handler for the "Remove photo" button.
 */
void AscentViewer::handle_removePhoto()
{
	removeCurrentPhoto();
}


// RIGHT CLICK

/**
 * Event handler for right clicks on the ascent info area.
 */
void AscentViewer::handle_rightClickOnAscentInfo(QPoint pos)
{
	popupInfoContextMenu(ascentInfoBox->mapToGlobal(pos));
}

/**
 * Event handler for right clicks on the peak info area.
 */
void AscentViewer::handle_rightClickOnPeakInfo(QPoint pos)
{
	popupInfoContextMenu(peakInfoBox->mapToGlobal(pos));
}

/**
 * Event handler for right clicks on the trip info area.
 */
void AscentViewer::handle_rightClickOnTripInfo(QPoint pos)
{
	popupInfoContextMenu(tripInfoBox->mapToGlobal(pos));
}


// EDIT ACTIONS

/**
 * Event handler for the button controlling ascent description editability.
 */
void AscentViewer::handle_descriptionEditableChanged()
{
	if (descriptionEditable) saveDescription();
	
	descriptionEditable = editDescriptionButton->isChecked();
	
	descriptionTextBrowser->setReadOnly(!descriptionEditable);
}

/**
 * Event handler for the button controlling photo description editability.
 */
void AscentViewer::handle_photoDescriptionEditableChanged()
{
	if (photoDescriptionEditable) {
		savePhotoDescription();
		photoDescriptionLabel->setText(photoDescriptionLineEdit->text());
	} else {
		photoDescriptionLineEdit->setText(photoDescriptionLabel->text());
	}
	
	photoDescriptionEditable = editPhotoDescriptionButton->isChecked();
	
	photoDescriptionLabel	->setVisible(!photoDescriptionEditable);
	photoDescriptionLineEdit->setVisible(photoDescriptionEditable);
}


/**
 * Event handler for the context menu action for editing the current ascent.
 */
void AscentViewer::handle_editAscent()
{
	BufferRowIndex oldBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	openEditAscentDialogAndStore(this, db, oldBufferRowIndex);
	handleChangesToUnderlyingData(oldBufferRowIndex);
}

/**
 * Event handler for the context menu action for editing the current ascent's peak.
 */
void AscentViewer::handle_editPeak()
{
	BufferRowIndex oldAscentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ValidItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(oldAscentBufferRowIndex).toInt();
	BufferRowIndex peakBufferRowIndex = db->peaksTable->getBufferIndexForPrimaryKey(peakID);
	openEditPeakDialogAndStore(this, db, peakBufferRowIndex);
	handleChangesToUnderlyingData(oldAscentBufferRowIndex);
}

/**
 * Event handler for the context menu action for editing the current ascent's trip.
 */
void AscentViewer::handle_editTrip()
{
	BufferRowIndex oldAscentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ValidItemID tripID = db->ascentsTable->tripIDColumn->getValueAt(oldAscentBufferRowIndex).toInt();
	BufferRowIndex tripBufferRowIndex = db->tripsTable->getBufferIndexForPrimaryKey(tripID);
	openEditTripDialogAndStore(this, db, tripBufferRowIndex);
	handleChangesToUnderlyingData(oldAscentBufferRowIndex);
}



/**
 * Spawns the info area context menu at the given position and controls the enabled state of its
 * actions.
 * 
 * @param pos	The position to spawn the context menu at.
 */
void AscentViewer::popupInfoContextMenu(QPoint pos)
{
	ItemID peakID = db->ascentsTable->peakIDColumn->getValueFor(currentAscentID.forceValid()).toInt();
	editPeakAction->setEnabled(peakID.isValid());
	ItemID tripID = db->ascentsTable->tripIDColumn->getValueFor(currentAscentID.forceValid()).toInt();
	editTripAction->setEnabled(tripID.isValid());
	
	infoContextMenu.popup(pos);
}

/**
 * Handles changes in the composite ascents table, i.e. changes in data, filtering or sorting.
 * 
 * If the current ascent is no longer in the table, the user is informed and the ascent viewer
 * navigates to the next ascent in the table or closes if the table is now empty.
 * 
 * @param currentBufferRowIndex	The buffer row index of the current ascent.
 */
void AscentViewer::handleChangesToUnderlyingData(BufferRowIndex currentBufferRowIndex)
{
	// Filtering and sorting may have changed, update view row index
	ViewRowIndex newViewRowIndex = compAscents->findViewRowIndexForBufferRow(currentBufferRowIndex);
	
	if (newViewRowIndex.isValid()) {	// Current ascent still in table
		changeToAscent(newViewRowIndex);
		return;
	}
	
	// Open ascent was filtered out
	QString title = tr("Ascent filtered");
	QString message = tr("As a result of these changes, the ascent is now filtered out of the table.")
			+ "\n" + tr("Clear or modify the active filters to see it again.");
	QMessageBox::information(this, title, message);
	
	int numberOfVisibleRows = compAscents->rowCount();
	if (numberOfVisibleRows < 1) {	// Filtered table now empty
		return reject();
	}
	
	newViewRowIndex = currentViewRowIndex;
	if (newViewRowIndex.isAboveRange(numberOfVisibleRows)) {
		newViewRowIndex = ViewRowIndex(numberOfVisibleRows - 1);
	}
	changeToAscent(newViewRowIndex);
}



// EXIT BEHAVIOUR

/**
 * Prepares the ascent viewer for closing by saving data and implicit settings.
 */
void AscentViewer::reject()
{
	saveDescription();
	savePhotoDescription();
	saveDialogGeometry(this, mainWindow, &Settings::ascentViewer_geometry);
	saveSplitterSizes();
	QDialog::reject();
}

/**
 * Saves the current splitter sizes to settings.
 */
void AscentViewer::saveSplitterSizes()
{
	QList<int> splitterSizes = centralSplitter->sizes();
	QStringList stringList;
	for (int size : splitterSizes) {
		stringList.append(QString::number(size));
	}
	Settings::ascentViewer_splitterSizes.set(stringList);
}


/**
 * Restores the splitter sizes from settings.
 */
void AscentViewer::restoreSplitterSizes()
{
	QStringList splitterSizeStrings = Settings::ascentViewer_splitterSizes.get();
	if (splitterSizeStrings.size() != centralSplitter->sizes().size()) {
		// Can't restore splitter sizes from settings
		if (!splitterSizeStrings.isEmpty()) {
			qDebug() << QString("Couldn't restore splitter sizes for ascent viewer: Expected %1 numbers, but got %2")
					.arg(centralSplitter->sizes().size()).arg(splitterSizeStrings.size());
		}
		Settings::ascentViewer_splitterSizes.clear();
		return;
	}
	
	QList<int> splitterSizes = QList<int>();
	for (const QString& sizeString : splitterSizeStrings) {
		bool conversionOk = false;
		int size = sizeString.toInt(&conversionOk);
		if (!conversionOk) {
			qDebug() << QString("Couldn't restore splitter sizes for ascent viewer: Value(s) couldn't be converted to int");
			Settings::ascentViewer_splitterSizes.clear();
			return;
		}
		splitterSizes.append(size);
	}
	
	centralSplitter->setSizes(splitterSizes);
}
