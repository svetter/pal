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
 * @file ascent_viewer.cpp
 * 
 * This file defines the AscentViewer class.
 */

#include "ascent_viewer.h"
#include "src/viewer/tab_behavior_mode.h"

#include <QRandomGenerator>
#include <QMessageBox>
#include <QWebEngineView>
#include <QWebEngineSettings>



/**
 * Creates a new AscentViewer.
 * 
 * @param parent		The application's main window.
 * @param db			The project's database.
 * @param typesHandler	The application's ItemTypesHandler.
 * @param viewRowIndex	The view row index of the ascent to open in the viewer.
 */
AscentViewer::AscentViewer(MainWindow* parent, Database& db, const ItemTypesHandler* typesHandler, ViewRowIndex viewRowIndex) :
	QDialog(parent),
	mainWindow(parent),
	db(db),
	typesHandler(typesHandler),
	compAscents	((CompositeAscentsTable&)	typesHandler->get(ItemTypeAscent).compTable),
	compPeaks	((CompositePeaksTable&)		typesHandler->get(ItemTypePeak).compTable),
	compTrips	((CompositeTripsTable&)		typesHandler->get(ItemTypeTrip).compTable),
	currentViewRowIndex(viewRowIndex),
	currentAscentID(ItemID()),
	currentPeakID(ItemID()),
	currentTripID(ItemID()),
	tripDescriptionEditable(false),
	ascentDescriptionEditable(false),
	infoContextMenu(QMenu(this)),
	goToRandomAscentShortcut(nullptr),
	descriptionSplitterSizes({}),
	lastClickedTabIndex(Settings::ascentViewer_lastClickedTabIndex.get())
{
	setupUi(this);
	
	imageWidget->supplyPointers(&db, &currentAscentID);
	gpxMapWidget->supplyPointers(&db, &currentAscentID);
	
	setWindowIcon(QIcon(":/icons/ico/ascent_viewer_multisize_square.ico"));
	setSizeGripEnabled(true);
	additionalUISetup();
	
	connectUI();
	restoreImplicitSettings();
	setupContextMenus();
	setupShortcuts();
	
	changeToAscent(viewRowIndex);
}

/**
 * Destroys the AscentViewer.
 */
AscentViewer::~AscentViewer()
{
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
	
	firstAscentButton		->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastAscentButton		->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	firstAscentOfPeakButton	->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastAscentOfPeakButton	->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	
	tripDescriptionEditable		= editTripDescriptionButton->isChecked();
	ascentDescriptionEditable	= editAscentDescriptionButton->isChecked();
	handle_tripDescriptionEditableChanged();
	handle_ascentDescriptionEditableChanged();
}

/**
 * Connects interactive UI elements to event handler functions.
 */
void AscentViewer::connectUI()
{
	// Ascent navigation
	connect(firstAscentButton,				&QToolButton::clicked,		this,	&AscentViewer::handle_firstAscent);
	connect(previousAscentButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_previousAscent);
	connect(nextAscentButton,				&QToolButton::clicked,		this,	&AscentViewer::handle_nextAscent);
	connect(lastAscentButton,				&QToolButton::clicked,		this,	&AscentViewer::handle_lastAscent);
	connect(goToRandomAscentLabel,			&QLabel::linkActivated,		this,	&AscentViewer::handle_randomAscent);
	connect(firstAscentOfPeakButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_firstAscentOfPeak);
	connect(previousAscentOfPeakButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_previousAscentOfPeak);
	connect(nextAscentOfPeakButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_nextAscentOfPeak);
	connect(lastAscentOfPeakButton,			&QToolButton::clicked,		this,	&AscentViewer::handle_lastAscentOfPeak);
	// Edit buttons
	connect(editTripDescriptionButton,		&QToolButton::clicked,		this,	&AscentViewer::handle_tripDescriptionEditableChanged);
	connect(editAscentDescriptionButton,	&QToolButton::clicked,		this,	&AscentViewer::handle_ascentDescriptionEditableChanged);
	// Context menus
	connect(tripInfoBox,					&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnTripInfo);
	connect(peakInfoBox,					&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnPeakInfo);
	connect(ascentInfoBox,					&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnAscentInfo);
	// Tabs
	connect(tabWidget,						&QTabWidget::tabBarClicked,	this,	&AscentViewer::handle_userClickedTab);
}

/**
 * Populates the info area context menu.
 */
void AscentViewer::setupContextMenus()
{
	editAscentAction	= infoContextMenu.addAction(db.ascentsTable.getEditItemString(),	this,	&AscentViewer::handle_editAscent);
	editPeakAction		= infoContextMenu.addAction(db.peaksTable.getEditItemString(),		this,	&AscentViewer::handle_editPeak);
	editTripAction		= infoContextMenu.addAction(db.tripsTable.getEditItemString(),		this,	&AscentViewer::handle_editTrip);
	
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
}



// ASCENT CHANGE

/**
 * Navigates to the ascent with the given view row index.
 * 
 * @param viewRowIndex	The view row index of the ascent to load.
 */
void AscentViewer::changeToAscent(ViewRowIndex viewRowIndex)
{
	imageWidget->ascentAboutToChange();
	gpxMapWidget->ascentAboutToChange();
	
	saveTripDescription();
	saveAscentDescription();
	
	// Get new IDs
	currentViewRowIndex	= viewRowIndex;
	const BufferRowIndex bufferRowIndex = compAscents.getBufferRowIndexForViewRow(viewRowIndex);
	currentAscentID = db.ascentsTable.getPrimaryKeyAt(bufferRowIndex);
	if (currentAscentID.isValid()) {
		currentPeakID = db.ascentsTable.peakIDColumn.getValueAt(bufferRowIndex);
		currentTripID = db.ascentsTable.tripIDColumn.getValueAt(bufferRowIndex);
	} else {
		currentPeakID = ItemID();
		currentTripID = ItemID();
	}
	
	// Update main window selection
	mainWindow->updateSelectionAfterUserAction(typesHandler->get(ItemTypeAscent), currentViewRowIndex);
	
	updateInfoArea();
	updateAscentNavigationTargets();
	updateAscentNavigationButtonsEnabled();
	updateAscentNavigationNumbers();
	updateTabLabels();

	switchTabIfIndicatedBySettings();
	
	imageWidget->ascentChanged();
	gpxMapWidget->ascentChanged();
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
	
	const BufferRowIndex ascentBufferRowIndex = compAscents.getBufferRowIndexForViewRow(currentViewRowIndex);
	
	if (currentTripID.isValid()) {
		BufferRowIndex tripBufferRowIndex = db.tripsTable.getBufferIndexForPrimaryKey(FORCE_VALID(currentTripID));
		tripNameLabel			->setText	(compAscents.tripColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		QString startDate					= compTrips.startDateColumn		.getFormattedValueAt(tripBufferRowIndex).toString();
		QString endDate						= compTrips.endDateColumn		.getFormattedValueAt(tripBufferRowIndex).toString();
		QString dateRange = startDate;
		if (startDate != endDate) {
			QString length = compTrips.lengthColumn.getFormattedValueAt(tripBufferRowIndex).toString();
			dateRange = startDate + " – " + endDate + " (" + length + (")");
		}
		tripDatesLabel			->setText	(dateRange);
	}
	
	if (currentPeakID.isValid()) {
		BufferRowIndex peakBufferRowIndex = db.peaksTable.getBufferIndexForPrimaryKey(FORCE_VALID(currentPeakID));
		peakNameLabel			->setText	(compAscents.peakColumn			.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakHeightLabel			->setText	(compAscents.peakHeightColumn	.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakVolcanoCheckbox		->setChecked(compAscents.volcanoColumn		.getRawValueAt(ascentBufferRowIndex).toBool());
		peakRegionLabel			->setText	(compAscents.regionColumn		.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakRangeLabel			->setText	(compAscents.rangeColumn		.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakCountryLabel		->setText	(compAscents.countryColumn		.getFormattedValueAt(ascentBufferRowIndex).toString());
		peakContinentLabel		->setText	(compAscents.continentColumn	.getFormattedValueAt(ascentBufferRowIndex).toString());
		const QString mapsLink				= db.peaksTable.mapsLinkColumn	.getValueAt(peakBufferRowIndex).toString();
		const QString earthLink				= db.peaksTable.earthLinkColumn	.getValueAt(peakBufferRowIndex).toString();
		const QString wikiLink				= db.peaksTable.wikiLinkColumn	.getValueAt(peakBufferRowIndex).toString();
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
	
	const QString ascentTitle				= compAscents.titleColumn		.getFormattedValueAt(ascentBufferRowIndex).toString();
	if (!ascentTitle.isEmpty()) {
		ascentTitleLabel		->setText	(compAscents.titleColumn		.getFormattedValueAt(ascentBufferRowIndex).toString());
		ascentTitleLabel		->setVisible(true);
		ascentInfoLine			->setVisible(true);
	}
	ascentDateLabel				->setText	(db.ascentsTable.dateColumn		.getValueAt				(ascentBufferRowIndex).toDate().toString("dd.MM.yyyy"));
	ascentTimeLabel				->setText	(db.ascentsTable.timeColumn		.getValueAt				(ascentBufferRowIndex).toTime().toString("HH:mm"));
	ascentPeakOnDayLabel		->setText	(db.ascentsTable.peakOnDayColumn.getValueAt				(ascentBufferRowIndex).toString() + ".");
	ascentElevationGainLabel	->setText	(compAscents.elevationGainColumn.getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentHikeKindLabel			->setText	(compAscents.hikeKindColumn		.getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentTraverseCheckbox		->setChecked(compAscents.traverseColumn		.getRawValueAt			(ascentBufferRowIndex).toBool());
	ascentDifficultyLabel		->setText	(compAscents.difficultyColumn	.getFormattedValueAt	(ascentBufferRowIndex).toString());
	ascentPeakOrdinalLabel		->setText	(compAscents.peakOrdinalColumn	.getFormattedValueAt	(ascentBufferRowIndex).toString());
	
	const QString hikersList = compAscents.hikersColumn.getFormattedValueAt(ascentBufferRowIndex).toString();
	if (!hikersList.isEmpty()) {
		ascentParticipantsLabel	->setText	(hikersList);
		ascentParticipantsBox->setVisible(true);
	}
	
	if (currentTripID.isValid()) {
		const QString tripDescription = db.tripsTable.descriptionColumn.getValueFor(FORCE_VALID(currentTripID)).toString();
		if (tripDescriptionEditable) {
			tripDescriptionTextBrowser->setPlainText(tripDescription);
		} else {
			tripDescriptionTextBrowser->setMarkdown(tripDescription);
		}
		// Restore description splitter
		if (!descriptionSplitterSizes.empty()) {
			if (descriptionSplitter->sizes().at(0) == 0) {
				descriptionSplitter->setSizes(descriptionSplitterSizes);
			}
			descriptionSplitterSizes.clear();
		}
	} else {
		tripDescriptionTextBrowser->clear();
		// Save sizes and collapse description splitter
		if (descriptionSplitterSizes.empty()) {
			descriptionSplitterSizes = descriptionSplitter->sizes();
			descriptionSplitter->setSizes({0, 1});
		}
	}
	tripDescriptionLabel		->setEnabled(currentTripID.isValid());
	editTripDescriptionButton	->setEnabled(currentTripID.isValid());
	tripDescriptionTextBrowser	->setEnabled(currentTripID.isValid());
	
	const QString ascentDescription = db.ascentsTable.descriptionColumn.getValueAt(ascentBufferRowIndex).toString();
	if (ascentDescriptionEditable) {
		ascentDescriptionTextBrowser->setPlainText(ascentDescription);
	} else {
		ascentDescriptionTextBrowser->setMarkdown(ascentDescription);
	}
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
	numShownAscents = compAscents.rowCount();
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
	
	if (currentPeakID.isValid()) {
		const QList<BufferRowIndex> matchingBufferRowIndices = db.ascentsTable.getMatchingBufferRowIndices(db.ascentsTable.peakIDColumn, currentPeakID.asQVariant());
		// Find matching view row indices (some or all ascents of the same peak may be filtered out)
		QList<ViewRowIndex> ascentOfPeakViewRowIndices = QList<ViewRowIndex>();
		for (const BufferRowIndex& matchingBufferRowIndex : matchingBufferRowIndices) {
			ViewRowIndex matchingViewRowIndex = compAscents.findViewRowIndexForBufferRow(matchingBufferRowIndex);
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

void AscentViewer::updateTabLabels()
{
	if (currentAscentID.isInvalid()) {
		imageTab->setWindowIconText(tr("Images"));
		mapTab	->setWindowIconText(tr("Map"));
		return;
	}
	const ValidItemID ascentId = FORCE_VALID(currentAscentID);
	
	const int numPhotos = db.photosTable.getPhotosForAscent(ascentId).count();
	const QString gpxFile = db.ascentsTable.gpxFileColumn.getValueFor(ascentId).toString();
	const QString gpxFileName = gpxFile.isNull() ? QString() : QFileInfo(gpxFile).fileName();
	
	const QString imageTabText	= numPhotos < 1		? tr("Images (none)")	: tr("Images (%1)")	.arg(numPhotos);
	const QString mapTabText	= gpxFile.isNull()	? tr("Map (none)")		: tr("Map (%1)")	.arg(gpxFileName);
	
	tabWidget->setTabText(tabWidget->indexOf(imageTab),	imageTabText);
	tabWidget->setTabText(tabWidget->indexOf(mapTab),	mapTabText);
}



// EDITING DESCRIPTIONS

/**
 * Saves the description for the current trip from the UI to the database.
 */
void AscentViewer::saveTripDescription()
{
	if (currentTripID.isInvalid() || !tripDescriptionEditable) return;
	
	const QString newDescription = tripDescriptionTextBrowser->toPlainText();
	const bool descriptionChanged = db.tripsTable.descriptionColumn.getValueFor(FORCE_VALID(currentTripID)) != newDescription;
	if (descriptionChanged && currentTripID.isValid()) {
		db.beginChangingData();
		db.tripsTable.updateCell(*this, FORCE_VALID(currentTripID), db.tripsTable.descriptionColumn, newDescription);
		db.finishChangingData();
	}
}

/**
 * Saves the description for the current ascent from the UI to the database.
 */
void AscentViewer::saveAscentDescription()
{
	if (currentAscentID.isInvalid() || !ascentDescriptionEditable) return;
	
	const QString newDescription = ascentDescriptionTextBrowser->toPlainText();
	const bool descriptionChanged = db.ascentsTable.descriptionColumn.getValueFor(FORCE_VALID(currentAscentID)) != newDescription;
	if (descriptionChanged) {
		db.beginChangingData();
		db.ascentsTable.updateCell(*this, FORCE_VALID(currentAscentID), db.ascentsTable.descriptionColumn, newDescription);
		db.finishChangingData();
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
	assert(firstAscentOfPeakViewRowIndex.isValid(numShownAscents));
	changeToAscent(firstAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to previous ascent of same peak" button.
 */
void AscentViewer::handle_previousAscentOfPeak()
{
	assert(currentAscentOfPeakIndex > 0 && currentAscentOfPeakIndex < numAscentsOfPeak);
	assert(previousAscentOfPeakViewRowIndex.isValid(numShownAscents));
	changeToAscent(previousAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to next ascent of same peak" button.
 */
void AscentViewer::handle_nextAscentOfPeak()
{
	assert(currentAscentOfPeakIndex >= 0 && currentAscentOfPeakIndex < numAscentsOfPeak - 1);
	assert(nextAscentOfPeakViewRowIndex.isValid(numShownAscents));
	changeToAscent(nextAscentOfPeakViewRowIndex);
}

/**
 * Event handler for the "Go to last ascent of same peak" button.
 */
void AscentViewer::handle_lastAscentOfPeak()
{
	assert(lastAscentOfPeakViewRowIndex.isValid(numShownAscents));
	changeToAscent(lastAscentOfPeakViewRowIndex);
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
 * Event handler for the button controlling trip description editability.
 */
void AscentViewer::handle_tripDescriptionEditableChanged()
{
	if (tripDescriptionEditable) saveTripDescription();
	
	tripDescriptionEditable = editTripDescriptionButton->isChecked();
	
	if (currentTripID.isInvalid()) return;
	if (tripDescriptionEditable) {
		const QString tripDescription = db.tripsTable.descriptionColumn.getValueFor(FORCE_VALID(currentTripID)).toString();
		tripDescriptionTextBrowser->setPlainText(tripDescription);
	} else {
		tripDescriptionTextBrowser->setMarkdown(tripDescriptionTextBrowser->toPlainText());
	}
	tripDescriptionTextBrowser->setReadOnly(!tripDescriptionEditable);
}

/**
 * Event handler for the button controlling ascent description editability.
 */
void AscentViewer::handle_ascentDescriptionEditableChanged()
{
	if (ascentDescriptionEditable) saveAscentDescription();
	
	ascentDescriptionEditable = editAscentDescriptionButton->isChecked();
	
	if (currentAscentID.isInvalid()) return;
	if (ascentDescriptionEditable) {
		const QString ascentDescription = db.ascentsTable.descriptionColumn.getValueFor(FORCE_VALID(currentAscentID)).toString();
		ascentDescriptionTextBrowser->setPlainText(ascentDescription);
	} else {
		ascentDescriptionTextBrowser->setMarkdown(ascentDescriptionTextBrowser->toPlainText());
	}
	ascentDescriptionTextBrowser->setReadOnly(!ascentDescriptionEditable);
}


/**
 * Event handler for the context menu action for editing the current ascent.
 */
void AscentViewer::handle_editAscent()
{
	gpxMapWidget->ascentAboutToChange();
	
	const BufferRowIndex oldAscentBufferRowIndex = compAscents.getBufferRowIndexForViewRow(currentViewRowIndex);
	
	openEditAscentDialogAndStore(*this, *mainWindow, db, oldAscentBufferRowIndex, [=](bool changesMade) {
		if (changesMade) {
			handleChangesToUnderlyingData(oldAscentBufferRowIndex);
			gpxMapWidget->ascentChanged();
		}
	});
}

/**
 * Event handler for the context menu action for editing the current ascent's peak.
 */
void AscentViewer::handle_editPeak()
{
	const BufferRowIndex oldAscentBufferRowIndex = compAscents.getBufferRowIndexForViewRow(currentViewRowIndex);
	const BufferRowIndex peakBufferRowIndex = db.peaksTable.getBufferIndexForPrimaryKey(FORCE_VALID(currentPeakID));
	
	openEditPeakDialogAndStore(*this, *mainWindow, db, peakBufferRowIndex, [=](bool changesMade) {
		if (changesMade) handleChangesToUnderlyingData(oldAscentBufferRowIndex);
	});
}

/**
 * Event handler for the context menu action for editing the current ascent's trip.
 */
void AscentViewer::handle_editTrip()
{
	const BufferRowIndex oldAscentBufferRowIndex = compAscents.getBufferRowIndexForViewRow(currentViewRowIndex);
	const BufferRowIndex tripBufferRowIndex = db.tripsTable.getBufferIndexForPrimaryKey(FORCE_VALID(currentTripID));
	
	openEditTripDialogAndStore(*this, *mainWindow, db, tripBufferRowIndex, [=](bool changesMade) {
		if (changesMade) handleChangesToUnderlyingData(oldAscentBufferRowIndex);
	});
}


// TABS

void AscentViewer::handle_userClickedTab(const int index)
{
	lastClickedTabIndex = index;
}



/**
 * Spawns the info area context menu at the given position and controls the enabled state of its
 * actions.
 * 
 * @param pos	The position to spawn the context menu at.
 */
void AscentViewer::popupInfoContextMenu(QPoint pos)
{
	editPeakAction->setEnabled(currentPeakID.isValid());
	editTripAction->setEnabled(currentTripID.isValid());
	
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
	ViewRowIndex newViewRowIndex = compAscents.findViewRowIndexForBufferRow(currentBufferRowIndex);
	
	if (newViewRowIndex.isValid()) {	// Current ascent still in table
		changeToAscent(newViewRowIndex);
		return;
	}
	
	// Open ascent was filtered out
	QString title = tr("Ascent filtered");
	QString message = tr("As a result of these changes, the ascent is now filtered out of the table.")
			+ "\n" + tr("Clear or modify the active filters to see it again.");
	QMessageBox::information(this, title, message);
	
	int numberOfVisibleRows = compAscents.rowCount();
	if (numberOfVisibleRows < 1) {	// Filtered table now empty
		return reject();
	}
	
	newViewRowIndex = currentViewRowIndex;
	if (newViewRowIndex.isAboveRange(numberOfVisibleRows)) {
		newViewRowIndex = ViewRowIndex(numberOfVisibleRows - 1);
	}
	changeToAscent(newViewRowIndex);
}

void AscentViewer::switchTabIfIndicatedBySettings()
{
	const QString behaviorSetting = Settings::ascentViewer_tabBehavior.get();
	const AscentViewerTabBehaviorMode behavior = AscentViewerTabBehaviorModeNames::parseAscentViewerTabBehaviorMode(behaviorSetting);
	if (behavior == AscentViewerTabBehaviorMode::Manual) return;
	
	const ValidItemID ascentID = FORCE_VALID(currentAscentID);
	const bool imagesPresent	= !db.photosTable.getPhotosForAscent(ascentID).isEmpty();
	const bool mapPresent		= !db.ascentsTable.gpxFileColumn.getValueFor(ascentID).toString().isEmpty();
	
	const int imageTabIndex		= tabWidget->indexOf(imageTab);
	const int mapTabIndex		= tabWidget->indexOf(mapTab);
	const int currentTabIndex	= tabWidget->currentIndex();
	
	const bool preferLastClicked = behavior == AscentViewerTabBehaviorMode::PreferLastClicked;
	const bool preferImages	= behavior == AscentViewerTabBehaviorMode::PreferImages	|| (preferLastClicked && lastClickedTabIndex == imageTabIndex);
	const bool preferMap	= behavior == AscentViewerTabBehaviorMode::PreferMap	|| (preferLastClicked && lastClickedTabIndex == mapTabIndex);
	const bool alwaysImages	= behavior == AscentViewerTabBehaviorMode::AlwaysImages;
	const bool alwaysMap	= behavior == AscentViewerTabBehaviorMode::AlwaysMap;
	
	if (currentTabIndex == imageTabIndex) {
		if (alwaysImages) return;
		if (preferImages && (imagesPresent || !mapPresent)) return;
		if (preferMap && !(mapPresent || !imagesPresent)) return;
		
		tabWidget->setCurrentIndex(mapTabIndex);
		return;
	}
	
	if (currentTabIndex == mapTabIndex) {
		if (alwaysMap) return;
		if (preferMap && (mapPresent || !imagesPresent)) return;
		if (preferImages && !(imagesPresent || !mapPresent)) return;
		
		tabWidget->setCurrentIndex(imageTabIndex);
		return;
	}
}



// EXIT BEHAVIOUR

/**
 * Prepares the ascent viewer for closing by saving data and implicit settings.
 */
void AscentViewer::reject()
{
	saveTripDescription();
	saveAscentDescription();
	saveImplicitSettings();
	imageWidget->aboutToExit();
	gpxMapWidget->aboutToExit();
	QDialog::reject();
}

/**
 * Saves all implicit settings for the ascent viewer to settings.
 */
void AscentViewer::saveImplicitSettings()
{
	saveDialogGeometry(*this, *mainWindow, Settings::ascentViewer_geometry);
	
	saveSplitterSizes( *leftSplitter, Settings::ascentViewer_leftSplitterSizes);
	saveSplitterSizes(*rightSplitter, Settings::ascentViewer_rightSplitterSizes);
	if (!descriptionSplitterSizes.empty()) {
		descriptionSplitter->setSizes(descriptionSplitterSizes);
	}
	saveSplitterSizes(*descriptionSplitter,	Settings::ascentViewer_descriptionSplitterSizes);
	
	Settings::ascentViewer_activeTabIndex.set(tabWidget->currentIndex());
	Settings::ascentViewer_lastClickedTabIndex.set(lastClickedTabIndex);
}

/**
 * Restores all implicit settings to the ascent viewer.
 */
void AscentViewer::restoreImplicitSettings()
{
	if (Settings::rememberWindowPositions.get()) {
		restoreDialogGeometry(*this, *mainWindow, Settings::ascentViewer_geometry);
	}
	
	restoreSplitterSizes(       *leftSplitter,	Settings::ascentViewer_leftSplitterSizes);
	restoreSplitterSizes(      *rightSplitter,	Settings::ascentViewer_rightSplitterSizes);
	restoreSplitterSizes(*descriptionSplitter,	Settings::ascentViewer_descriptionSplitterSizes);
	
	tabWidget->setCurrentIndex(Settings::ascentViewer_activeTabIndex.get());
	lastClickedTabIndex = Settings::ascentViewer_lastClickedTabIndex.get();
	
	imageWidget->restoreImplicitSettings();
}
