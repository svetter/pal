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
 * @file ascent_viewer.cpp
 * 
 * This file defines the AscentViewer class.
 */

#include "ascent_viewer.h"

#include "src/tools/relocate_photos_dialog.h"

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
	slideshowTimer(QTimer(this)),
	slideshowRunning(false),
	descriptionEditable(false),
	photoDescriptionEditable(false),
	infoContextMenu(QMenu(this)),
	goToRandomAscentShortcut(nullptr)
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/ascent_viewer_multisize_square.ico"));
	additionalUISetup();
	
	connectUI();
	restoreImplicitSettings();
	setupContextMenus();
	setupShortcuts();
	setupSlideshow();
	
	changeToAscent(viewRowIndex);
}

/**
 * Destroys the AscentViewer.
 */
AscentViewer::~AscentViewer()
{
	delete imageLabel;
	delete goToRandomAscentShortcut;
}



// INITIAL SETUP

/**
 * Configures stretch, restores geometry, sets button icons, and more UI setup.
 */
void AscentViewer::additionalUISetup()
{
	// Set spinner stretch factors as pairs of index and stretch
	leftSplitter	->setStretchFactor(0, 0);
	leftSplitter	->setStretchFactor(1, 1);
	rightSplitter	->setStretchFactor(0, 1);
	rightSplitter	->setStretchFactor(1, 2);
	rightSplitter	->setSizes({ rightSplitter->size().width() / 2, rightSplitter->size().width() / 2 });
	
	
	// Move ascent title label if settings dictate
	if (Settings::ascentViewer_ascentTitleUnderPeakName.get()) {
		peakInfoBoxLayout->insertWidget(1, ascentInfoLine);
		peakInfoBoxLayout->insertWidget(2, ascentTitleLabel);
	}
	
	
	// Set icons for info boxes
	tripInfoBox				->setIcon(QIcon(":/icons/trip.svg"),	28, 28);
	peakInfoBox				->setIcon(QIcon(":/icons/peak.svg"),	28, 28);
	ascentInfoBox			->setIcon(QIcon(":/icons/ascent.svg"),	28, 28);
	ascentParticipantsBox	->setIcon(QIcon(":/icons/hiker.svg"),	20, 20);
	
	peakVolcanoCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
	ascentTraverseCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
	
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
	
	// Make QGroupBox titles turn gray when disabled (like on other widgets)
	QColor disabledColor = QApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
	QPalette disabledPalette = QPalette();
	disabledPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
	slideshowBox->setPalette(disabledPalette);
}

/**
 * Connects interactive UI elements to event handler functions.
 */
void AscentViewer::connectUI()
{
	// Ascent navigation
	connect(firstAscentButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_firstAscent);
	connect(previousAscentButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_previousAscent);
	connect(nextAscentButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_nextAscent);
	connect(lastAscentButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_lastAscent);
	connect(goToRandomAscentLabel,		&QLabel::linkActivated,		this,	&AscentViewer::handle_randomAscent);
	connect(firstAscentOfPeakButton,	&QToolButton::clicked,		this,	&AscentViewer::handle_firstAscentOfPeak);
	connect(previousAscentOfPeakButton,	&QToolButton::clicked,		this,	&AscentViewer::handle_previousAscentOfPeak);
	connect(nextAscentOfPeakButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_nextAscentOfPeak);
	connect(lastAscentOfPeakButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_lastAscentOfPeak);
	// Photo navigation
	connect(firstPhotoButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_firstPhoto);
	connect(previousPhotoButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_previousPhoto);
	connect(nextPhotoButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_nextPhoto);
	connect(lastPhotoButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_lastPhoto);
	// Slideshow
	connect(slideshowStartStopButton,	&QToolButton::clicked,		this,	&AscentViewer::handle_toggleSlideshow);
	connect(slideshowIntervalSpinner,	&QSpinBox::valueChanged,	this,	&AscentViewer::handle_slideshowIntervalChanged);
	// Changing photos
	connect(movePhotoLeftButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_movePhotoLeft);
	connect(movePhotoRightButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_movePhotoRight);
	connect(addPhotosButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_addPhotos);
	connect(removePhotoButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_removePhoto);
	// Image file error box
	connect(imageErrorRemoveButton,		&QPushButton::clicked,		this,	&AscentViewer::handle_removePhoto);
	connect(imageErrorReplaceButton,	&QPushButton::clicked,		this,	&AscentViewer::handle_replacePhoto);
	connect(imageErrorRelocateButton,	&QPushButton::clicked,		this,	&AscentViewer::handle_relocatePhotos);
	// Edit buttons
	connect(editDescriptionButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_descriptionEditableChanged);
	connect(editPhotoDescriptionButton,	&QToolButton::clicked,		this,	&AscentViewer::handle_photoDescriptionEditableChanged);
	// Context menus
	connect(tripInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnTripInfo);
	connect(peakInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnPeakInfo);
	connect(ascentInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnAscentInfo);
	// Drag and drop
	connect(imageFrame,				&FileDropFrame::filesDropped,	this,	&AscentViewer::handle_filesDropped);
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
	
	goToRandomAscentShortcut = new QShortcut(QKeySequence(Qt::Key_R), this, SLOT(handle_randomAscent()));
	
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

/**
 * Prepares slideshow functionality.
 */
void AscentViewer::setupSlideshow()
{
	connect(&slideshowTimer, &QTimer::timeout, this, &AscentViewer::handle_slideshowTimerTrigger);
	connect(imageLabel, &ScalableImageLabel::userInteracted, this, &AscentViewer::handle_userInteractedWithImageLabel);
}



// ASCENT CHANGE

/**
 * Navigates to the ascent with the given view row index.
 * 
 * @param viewRowIndex	The view row index of the ascent to load.
 */
void AscentViewer::changeToAscent(ViewRowIndex viewRowIndex)
{
	stopSlideshow();
	
	saveDescription();
	savePhotoDescription();
	
	currentViewRowIndex	= viewRowIndex;
	BufferRowIndex bufferRowIndex = compAscents->getBufferRowIndexForViewRow(viewRowIndex);
	currentAscentID = db->ascentsTable.getPrimaryKeyAt(bufferRowIndex);
	// Update main window selection
	mainWindow->updateSelectionAfterUserAction(typesHandler->get(ItemTypeAscent), currentViewRowIndex);
	
	updateInfoArea();
	loadPhotosList();
	changeToPhoto(photos.isEmpty() ? -1 : 0, false);
	updateAscentNavigationTargets();
	updateAscentNavigationButtonsEnabled();
	updateAscentNavigationNumbers();
	
	if (slideshowAutostartCheckbox->isChecked() && photos.size() > 1) {
		startSlideshow(false);
	}
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
	
	ItemID tripID = db->ascentsTable.tripIDColumn.getValueAt(ascentBufferRowIndex);
	if (tripID.isValid()) {
		BufferRowIndex tripBufferRowIndex = db->tripsTable.getBufferIndexForPrimaryKey(FORCE_VALID(tripID));
		tripNameLabel			->setText	(compAscents->tripColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		QString startDate					= compTrips->startDateColumn		.getFormattedValueAt(tripBufferRowIndex).toString();
		QString endDate						= compTrips->endDateColumn			.getFormattedValueAt(tripBufferRowIndex).toString();
		QString dateRange = startDate;
		if (startDate != endDate) {
			QString length = compTrips->lengthColumn.getFormattedValueAt(tripBufferRowIndex).toString();
			dateRange = startDate + " – " + endDate + " (" + length + (")");
		}
		tripDatesLabel			->setText	(dateRange);
	}
	
	ItemID peakID = db->ascentsTable.peakIDColumn.getValueAt(ascentBufferRowIndex);
	if (peakID.isValid()) {
		BufferRowIndex peakBufferRowIndex = db->peaksTable.getBufferIndexForPrimaryKey(FORCE_VALID(peakID));
		peakNameLabel			->setText	(compAscents->peakColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakHeightLabel			->setText	(compAscents->peakHeightColumn		.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakVolcanoCheckbox		->setChecked(compAscents->volcanoColumn			.getRawValueAt(ascentBufferRowIndex).toBool());
		peakRegionLabel			->setText	(compAscents->regionColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakRangeLabel			->setText	(compAscents->rangeColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakCountryLabel		->setText	(compAscents->countryColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakContinentLabel		->setText	(compAscents->continentColumn		.getFormattedValueAt(ascentBufferRowIndex).toString());
		QString mapsLink					= db->peaksTable.mapsLinkColumn		.getValueAt(peakBufferRowIndex).toString();
		QString earthLink					= db->peaksTable.earthLinkColumn	.getValueAt(peakBufferRowIndex).toString();
		QString wikiLink					= db->peaksTable.wikiLinkColumn		.getValueAt(peakBufferRowIndex).toString();
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
	
	QString ascentTitle						= compAscents->titleColumn			.getFormattedValueAt(ascentBufferRowIndex).toString();
	if (!ascentTitle.isEmpty()) {
		ascentTitleLabel		->setText	(compAscents->titleColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		ascentTitleLabel		->setVisible(true);
		ascentInfoLine			->setVisible(true);
	}
	ascentDateLabel				->setText	(db->ascentsTable.dateColumn		.getValueAt			(ascentBufferRowIndex).toDate().toString("dd.MM.yyyy"));
	ascentTimeLabel				->setText	(db->ascentsTable.timeColumn		.getValueAt			(ascentBufferRowIndex).toTime().toString("HH:mm"));
	ascentPeakOnDayLabel		->setText	(db->ascentsTable.peakOnDayColumn	.getValueAt			(ascentBufferRowIndex).toString() + ".");
	ascentElevationGainLabel	->setText	(compAscents->elevationGainColumn	.getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentHikeKindLabel			->setText	(compAscents->hikeKindColumn		.getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentTraverseCheckbox		->setChecked(compAscents->traverseColumn		.getRawValueAt			(ascentBufferRowIndex).toBool());
	ascentDifficultyLabel		->setText	(compAscents->difficultyColumn		.getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentPeakOrdinalLabel		->setText	(compAscents->peakOrdinalColumn		.getFormattedValueAt	(ascentBufferRowIndex).toString());
	
	QString hikersList = compAscents->hikersColumn.getFormattedValueAt(ascentBufferRowIndex).toString();
	if (!hikersList.isEmpty()) {
		ascentParticipantsLabel	->setText	(hikersList);
		ascentParticipantsBox->setVisible(true);
	}
	
	descriptionTextBrowser->setText(db->ascentsTable.descriptionColumn.getValueAt(ascentBufferRowIndex).toString());
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
	numShownAscents = compAscents->rowCount();
	const ViewRowIndex minViewRowIndex	= ViewRowIndex(0);
	const ViewRowIndex maxViewRowIndex	= ViewRowIndex(numShownAscents - 1);
	
	firstAscentViewRowIndex		= (currentViewRowIndex == minViewRowIndex) ? ViewRowIndex() : minViewRowIndex;
	previousAscentViewRowIndex	= (currentViewRowIndex == minViewRowIndex) ? ViewRowIndex() : (currentViewRowIndex - 1);
	nextAscentViewRowIndex		= (currentViewRowIndex == maxViewRowIndex) ? ViewRowIndex() : (currentViewRowIndex + 1);
	lastAscentViewRowIndex		= (currentViewRowIndex == maxViewRowIndex) ? ViewRowIndex() : maxViewRowIndex;
	
	firstAscentOfPeakViewRowIndex		= ViewRowIndex();
	previousAscentOfPeakViewRowIndex	= ViewRowIndex();
	nextAscentOfPeakViewRowIndex		= ViewRowIndex();
	lastAscentOfPeakViewRowIndex		= ViewRowIndex();
	
	currentAscentOfPeakIndex	= -1;
	numAscentsOfPeak			= 0;
	
	BufferRowIndex bufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ItemID peakID = db->ascentsTable.peakIDColumn.getValueAt(bufferRowIndex);
	if (peakID.isValid()) {
		QList<BufferRowIndex> matchingBufferRowIndices = db->ascentsTable.getMatchingBufferRowIndices(db->ascentsTable.peakIDColumn, peakID.asQVariant());
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
	QString allAscentsNewText = QString::number(currentViewRowIndex.get() + 1) + " / " + QString::number(numShownAscents);
	allAscentsNumberLabel->setText(allAscentsNewText);
	allAscentsNumberLabel->setEnabled(numShownAscents > 0);
	
	QString peakAscentsNewText = QString::number(currentAscentOfPeakIndex + 1) + " / " + QString::number(numAscentsOfPeak);
	ascentOfPeakNumberLabel->setText(peakAscentsNewText);
	ascentOfPeakNumberLabel->setEnabled(numAscentsOfPeak > 1);
}

/**
 * Loads the current state of the photo list for the current ascent from the database.
 * 
 * Does not change or reset currentPhotoIndex.
 */
void AscentViewer::loadPhotosList()
{
	photos.clear();
	
	QList<Photo> savedPhotos = db->photosTable.getPhotosForAscent(FORCE_VALID(currentAscentID));
	if (!savedPhotos.isEmpty()) {
		photos = savedPhotos;
	}
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
			AscentViewer::imageErrorMessageOccurred(trimmedMessage);
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
void AscentViewer::updateImageFrameProperties(bool imagePresent, bool imageReadable)
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
void AscentViewer::startSlideshow(bool nextPhotoImmediately)
{
	if (slideshowRunning) return;
	
	if (photoDescriptionEditable) {
		editPhotoDescriptionButton->setChecked(false);
		handle_photoDescriptionEditableChanged();
	}
	
	slideshowTimer.start(slideshowIntervalSpinner->value() * 1000);
	slideshowStartStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	slideshowRunning = true;
	
	if (nextPhotoImmediately) handle_slideshowTimerTrigger();
}

/**
 * Stops the slideshow if it is running.
 * 
 * Changes the slideshow button icon and stops the timer.
 */
void AscentViewer::stopSlideshow()
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
void AscentViewer::restartSlideshowTimerIfRunning()
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
void AscentViewer::addPhotosFromDialog()
{
	QString preSelectedDir = QString();
	if (!photos.isEmpty()) {
		assert(currentPhotoIndex >= 0);
		QFileInfo(photos.at(currentPhotoIndex).filepath).path();
	}
	QStringList filepaths = openFileDialogForMultiPhotoSelection(this, preSelectedDir);
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
void AscentViewer::addPhotos(QStringList filepaths)
{
	if (filepaths.isEmpty()) return;
	
	savePhotoDescription();
	
	if (currentPhotoIndex < 0) currentPhotoIndex = -1;
	currentPhotoIndex++;	// Set to index of first inserted photo
	for (int i = 0; i < filepaths.size(); i++) {
		photos.insert(currentPhotoIndex + i, Photo(currentAscentID, ItemID(), -1, filepaths.at(i), QString()));
	}
	savePhotosList();
	
	changeToPhoto(currentPhotoIndex, false);
}

/**
 * Removes the current photo from the current ascent and displays the next one, if present.
 */
void AscentViewer::removeCurrentPhoto()
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
void AscentViewer::replaceCurrentPhoto()
{
	savePhotoDescription();
	
	/*: %1 is a filepath, so it is best if it remains at the end of the string. */
	QString dialogTitle = tr("Replace %1").arg(photos.at(currentPhotoIndex).filepath);
	QString preSelectedDir = QFileInfo(photos.at(currentPhotoIndex).filepath).path();
	QString filepath = openFileDialogForSinglePhotoSelection(this, preSelectedDir, dialogTitle);
	if (filepath.isEmpty()) return;
	
	photos[currentPhotoIndex].filepath = filepath;
	savePhotosList();
	
	changeToPhoto(currentPhotoIndex, false);
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
	db->photosTable.updateRows(this, FORCE_VALID(currentAscentID), photos);
}



// EDITING DESCRIPTION

/**
 * Saves the description for the current ascent from the UI to the database.
 */
void AscentViewer::saveDescription()
{
	if (currentAscentID.isInvalid() || !descriptionEditable) return;
	
	QString newDescription = descriptionTextBrowser->toPlainText();
	bool descriptionChanged = db->ascentsTable.descriptionColumn.getValueFor(FORCE_VALID(currentAscentID)) != newDescription;
	if (descriptionChanged) {
		db->ascentsTable.updateCell(this, FORCE_VALID(currentAscentID), db->ascentsTable.descriptionColumn, newDescription);
	}
}



// === UI EVENT HANDLERS ===

// ASCENT NAVIGATION

/**
 * Event handler for the "Go to first ascent" button.
 */
void AscentViewer::handle_firstAscent()
{
	assert(firstAscentViewRowIndex.isValid(numShownAscents));
	changeToAscent(firstAscentViewRowIndex);
}

/**
 * Event handler for the "Go to previous ascent" button.
 */
void AscentViewer::handle_previousAscent()
{
	assert(previousAscentViewRowIndex.isValid(numShownAscents));
	changeToAscent(previousAscentViewRowIndex);
}

/**
 * Event handler for the "Go to next ascent" button.
 */
void AscentViewer::handle_nextAscent()
{
	assert(nextAscentViewRowIndex.isValid(numShownAscents));
	changeToAscent(nextAscentViewRowIndex);
}

/**
 * Event handler for the "Go to last ascent" button.
 */
void AscentViewer::handle_lastAscent()
{
	assert(lastAscentViewRowIndex.isValid(numShownAscents));
	changeToAscent(lastAscentViewRowIndex);
}

/**
 * Event handler for the "Go to random ascent" button.
 */
void AscentViewer::handle_randomAscent()
{
	QRandomGenerator rand = QRandomGenerator();
	rand.seed(QDateTime::currentMSecsSinceEpoch());
	// Generate random ViewRowIndex between 0 and numShownAscents - 2 (excluding the last ascent)
	ViewRowIndex randomIndex = ViewRowIndex(rand.bounded(numShownAscents - 1));
	// If the random index is the same as the current one, use the last ascent instead
	if (randomIndex == currentViewRowIndex) {
		randomIndex = ViewRowIndex(numShownAscents - 1);
	}
	changeToAscent(ViewRowIndex(randomIndex));
}

/**
 * Event handler for the "Go to first ascent of same peak" button.
 */
void AscentViewer::handle_firstAscentOfPeak()
{
	assert(firstAscentOfPeakViewRowIndex.isValid(numAscentsOfPeak));
	changeToAscent(firstAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to previous ascent of same peak" button.
 */
void AscentViewer::handle_previousAscentOfPeak()
{
	assert(previousAscentOfPeakViewRowIndex.isValid(numAscentsOfPeak));
	changeToAscent(previousAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to next ascent of same peak" button.
 */
void AscentViewer::handle_nextAscentOfPeak()
{
	assert(nextAscentOfPeakViewRowIndex.isValid(numAscentsOfPeak));
	changeToAscent(nextAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to last ascent of same peak" button.
 */
void AscentViewer::handle_lastAscentOfPeak()
{
	assert(lastAscentOfPeakViewRowIndex.isValid(numAscentsOfPeak));
	changeToAscent(lastAscentOfPeakViewRowIndex);
}


// PHOTO NAVIGATION

/**
 * Event handler for the "Go to first photo" button.
 */
void AscentViewer::handle_firstPhoto()
{
	changeToPhoto(0, true);
	restartSlideshowTimerIfRunning();
}

/**
 * Event handler for the "Go to previous photo" button.
 */
void AscentViewer::handle_previousPhoto()
{
	changeToPhoto(currentPhotoIndex - 1, true);
	restartSlideshowTimerIfRunning();
}

/**
 * Event handler for the "Go to next photo" button.
 */
void AscentViewer::handle_nextPhoto()
{
	changeToPhoto(currentPhotoIndex + 1, true);
	restartSlideshowTimerIfRunning();
}

/**
 * Event handler for the "Go to last photo" button.
 */
void AscentViewer::handle_lastPhoto()
{
	changeToPhoto(photos.size() - 1, true);
	restartSlideshowTimerIfRunning();
}


// SLIDESHOW

/**
 * Starts the slideshow if it is not running and stops it otherwise.
 */
void AscentViewer::handle_toggleSlideshow()
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
void AscentViewer::handle_slideshowTimerTrigger()
{
	if (photos.size() < 2) return;
	int nextPhotoIndex = currentPhotoIndex + 1;
	if (nextPhotoIndex >= photos.size()) nextPhotoIndex = 0;
	changeToPhoto(nextPhotoIndex, true);
}

/**
 * Event handler for changes in the slideshow interval spin box.
 */
void AscentViewer::handle_slideshowIntervalChanged()
{
	slideshowTimer.setInterval(slideshowIntervalSpinner->value() * 1000);
}

/**
 * Event handler for user interaction with the image area.
 */
void AscentViewer::handle_userInteractedWithImageLabel()
{
	stopSlideshow();
}


// CHANGING PHOTOS

/**
 * Event handler for the "Move photo left" button.
 */
void AscentViewer::handle_movePhotoLeft()
{
	restartSlideshowTimerIfRunning();
	moveCurrentPhoto(true);
}

/**
 * Event handler for the "Move photo right" button.
 */
void AscentViewer::handle_movePhotoRight()
{
	restartSlideshowTimerIfRunning();
	moveCurrentPhoto(false);
}

/**
 * Event handler for the "Add photos" button.
 */
void AscentViewer::handle_addPhotos()
{
	stopSlideshow();
	addPhotosFromDialog();
}

/**
 * Event handler for the "Remove photo" buttons.
 */
void AscentViewer::handle_removePhoto()
{
	removeCurrentPhoto();
}

/**
 * Event handler for the "Replace" button in the file not found box.
 */
void AscentViewer::handle_replacePhoto()
{
	stopSlideshow();
	replaceCurrentPhoto();
}

/**
 * Event handler for the "Mass relocate" button in the file not found box.
 */
void AscentViewer::handle_relocatePhotos()
{
	savePhotoDescription();
	stopSlideshow();
	RelocatePhotosDialog(this, db).exec();
	loadPhotosList();
	changeToPhoto(currentPhotoIndex, false);
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
		
		stopSlideshow();
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
	openEditAscentDialogAndStore(this, mainWindow, db, oldBufferRowIndex);
	handleChangesToUnderlyingData(oldBufferRowIndex);
}

/**
 * Event handler for the context menu action for editing the current ascent's peak.
 */
void AscentViewer::handle_editPeak()
{
	BufferRowIndex oldAscentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ValidItemID peakID = VALID_ITEM_ID(db->ascentsTable.peakIDColumn.getValueAt(oldAscentBufferRowIndex).toInt());
	BufferRowIndex peakBufferRowIndex = db->peaksTable.getBufferIndexForPrimaryKey(peakID);
	openEditPeakDialogAndStore(this, mainWindow, db, peakBufferRowIndex);
	handleChangesToUnderlyingData(oldAscentBufferRowIndex);
}

/**
 * Event handler for the context menu action for editing the current ascent's trip.
 */
void AscentViewer::handle_editTrip()
{
	BufferRowIndex oldAscentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ValidItemID tripID = VALID_ITEM_ID(db->ascentsTable.tripIDColumn.getValueAt(oldAscentBufferRowIndex).toInt());
	BufferRowIndex tripBufferRowIndex = db->tripsTable.getBufferIndexForPrimaryKey(tripID);
	openEditTripDialogAndStore(this, mainWindow, db, tripBufferRowIndex);
	handleChangesToUnderlyingData(oldAscentBufferRowIndex);
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
void AscentViewer::handle_filesDropped(QStringList filepaths)
{
	QStringList checkedPaths = checkFilepathsAndAskUser(this, filepaths);
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
void AscentViewer::imageErrorMessageOccurred(const QString& message)
{
	imageLoadErrorMessage = message;
}



/**
 * Spawns the info area context menu at the given position and controls the enabled state of its
 * actions.
 * 
 * @param pos	The position to spawn the context menu at.
 */
void AscentViewer::popupInfoContextMenu(QPoint pos)
{
	ItemID peakID = db->ascentsTable.peakIDColumn.getValueFor(FORCE_VALID(currentAscentID)).toInt();
	editPeakAction->setEnabled(peakID.isValid());
	ItemID tripID = db->ascentsTable.tripIDColumn.getValueFor(FORCE_VALID(currentAscentID)).toInt();
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
	saveImplicitSettings();
	QDialog::reject();
}

/**
 * Saves all implicit settings for the ascent viewer to settings.
 */
void AscentViewer::saveImplicitSettings()
{
	saveDialogGeometry(this, mainWindow, &Settings::ascentViewer_geometry);
	
	saveSplitterSizes( leftSplitter, &Settings::ascentViewer_leftSplitterSizes);
	saveSplitterSizes(rightSplitter, &Settings::ascentViewer_rightSplitterSizes);
	
	Settings::ascentViewer_slideshowInterval.set(slideshowIntervalSpinner->value());
	Settings::ascentViewer_slideshowAutostart.set(slideshowAutostartCheckbox->isChecked());
}

/**
 * Restores all implicit settings to the ascent viewer.
 */
void AscentViewer::restoreImplicitSettings()
{
	if (Settings::rememberWindowPositions.get()) {
		restoreDialogGeometry(this, mainWindow, &Settings::ascentViewer_geometry);
	}
	
	restoreSplitterSizes( leftSplitter, &Settings::ascentViewer_leftSplitterSizes);
	restoreSplitterSizes(rightSplitter, &Settings::ascentViewer_rightSplitterSizes);
	
	slideshowIntervalSpinner->setValue(Settings::ascentViewer_slideshowInterval.get());
	slideshowAutostartCheckbox->setChecked(Settings::ascentViewer_slideshowAutostart.get());
}
