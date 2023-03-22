#include "ascent_viewer.h"

#include <QDialog>
#include <QStyle>
#include <QImageReader>
#include <QMessageBox>
#include <QDir>
#include <QColorSpace>
#include <QFileDialog>



AscentViewer::AscentViewer(MainWindow* parent, Database* db, const ItemTypesHandler* typesHandler, int viewRowIndex) :
		QDialog(parent),
		mainWindow(parent),
		db(db),
		typesHandler(typesHandler),
		compAscents((CompositeAscentsTable*) typesHandler->get(Ascent)->compTable),
		compPeaks((CompositePeaksTable*) typesHandler->get(Peak)->compTable),
		compTrips((CompositeTripsTable*) typesHandler->get(Trip)->compTable),
		currentViewRowIndex(viewRowIndex),
		currentAscentID(ItemID()),
		photos(QList<Photo>()),
		descriptionEditable(false),
		photoDescriptionEditable(false),
		infoContextMenu(QMenu(this))
{
	setupUi(this);
	additionalUISetup();
	
	connectUI();
	setupContextMenus();
	setupShortcuts();
	
	changeToAscent(viewRowIndex);
}

AscentViewer::~AscentViewer()
{}



// INITIAL SETUP

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
	
	imageDisplay = new ImageDisplay(imageFrame);
	imageFrameLayout->addWidget(imageDisplay);
	
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

void AscentViewer::setupContextMenus()
{
	editAscentAction	= infoContextMenu.addAction(tr("Edit ascent..."),	this,	&AscentViewer::handle_editAscent);
	editPeakAction		= infoContextMenu.addAction(tr("Edit peak..."),		this,	&AscentViewer::handle_editPeak);
	editTripAction		= infoContextMenu.addAction(tr("Edit trip..."),		this,	&AscentViewer::handle_editTrip);
}

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

void AscentViewer::changeToAscent(int viewRowIndex)
{
	saveDescription();
	savePhotoDescription();
	
	currentViewRowIndex	= viewRowIndex;
	int bufferRowIndex	= compAscents->getBufferRowIndexForViewRow(viewRowIndex);
	currentAscentID		= db->ascentsTable->getPrimaryKeyAt(bufferRowIndex);
	// Update main window selection
	mainWindow->updateSelectionAfterUserAction(*typesHandler->get(Ascent), currentViewRowIndex);
	
	updateAscentInfo();
	setupPhotos();
	updateAscentNavigationTargets();
	updateAscentNavigationButtonsEnabled();
}

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

void AscentViewer::updateAscentInfo()
{
	resetInfoLabels();
	
	int ascentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	
	ItemID tripID = db->ascentsTable->tripIDColumn->getValueAt(ascentBufferRowIndex);
	if (tripID.isValid()) {
		int tripBufferRowIndex = db->tripsTable->getBufferIndexForPrimaryKey(tripID.forceValid());
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
		int peakBufferRowIndex = db->peaksTable->getBufferIndexForPrimaryKey(peakID.forceValid());
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
	ascentDateLabel				->setText	(compAscents->dateColumn			->getFormattedValueAt(ascentBufferRowIndex).toString());
	ascentTimeLabel				->setText	(db->ascentsTable->timeColumn		->getValueAt(ascentBufferRowIndex).toString());
	ascentPeakOnDayLabel		->setText	(db->ascentsTable->peakOnDayColumn	->getValueAt(ascentBufferRowIndex).toString() + ".");
	ascentElevationGainLabel	->setText	(compAscents->elevationGainColumn	->getFormattedValueAt(ascentBufferRowIndex).toString());
	ascentHikeKindLabel			->setText	(compAscents->hikeKindColumn		->getFormattedValueAt(ascentBufferRowIndex).toString());
	ascentTraverseCheckbox		->setChecked(compAscents->traverseColumn		->getRawValueAt(ascentBufferRowIndex).toBool());
	ascentDifficultyLabel		->setText	(compAscents->difficultyColumn		->getFormattedValueAt(ascentBufferRowIndex).toString());
	QString hikersList = compAscents->hikersColumn->getFormattedValueAt(ascentBufferRowIndex).toString();
	if (!hikersList.isEmpty()) {
		ascentParticipantsLabel	->setText	(hikersList);
		ascentParticipantsBox->setVisible(true);
	}
	
	descriptionTextBrowser		->setText	(db->ascentsTable->descriptionColumn->getValueAt(ascentBufferRowIndex).toString());
}

void AscentViewer::updateAscentNavigationTargets()
{
	int minViewRowIndex	= 0;
	int maxViewRowIndex	= compAscents->rowCount() - 1;
	
	firstAscentViewRowIndex		= currentViewRowIndex == minViewRowIndex ? -1 : minViewRowIndex;
	previousAscentViewRowIndex	= currentViewRowIndex == minViewRowIndex ? -1 : (currentViewRowIndex - 1);
	nextAscentViewRowIndex		= currentViewRowIndex == maxViewRowIndex ? -1 : (currentViewRowIndex + 1);
	lastAscentViewRowIndex		= currentViewRowIndex == maxViewRowIndex ? -1 : maxViewRowIndex;
	
	firstAscentOfPeakViewRowIndex		= -1;
	previousAscentOfPeakViewRowIndex	= -1;
	nextAscentOfPeakViewRowIndex		= -1;
	lastAscentOfPeakViewRowIndex		= -1;
	
	int bufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(bufferRowIndex);
	if (peakID.isValid()) {
		QList<int> matchingBufferRowIndices = db->ascentsTable->getMatchingBufferRowIndices(db->ascentsTable->peakIDColumn, peakID.asQVariant());
		QList<int> ascentOfPeakViewRowIndices = QList<int>();
		for (int matchingBufferRowIndex : matchingBufferRowIndices) {
			int matchingViewRowIndex = compAscents->findCurrentViewRowIndex(matchingBufferRowIndex);
			if (matchingViewRowIndex < 0) continue;
			ascentOfPeakViewRowIndices.append(matchingViewRowIndex);
		}
		assert(!ascentOfPeakViewRowIndices.isEmpty());
		
		int minAscentOfPeakViewRowIndex	= ascentOfPeakViewRowIndices.first();
		int maxAscentOfPeakViewRowIndex	= ascentOfPeakViewRowIndices.last();
		
		if (currentViewRowIndex > minAscentOfPeakViewRowIndex) {
			firstAscentOfPeakViewRowIndex		= minAscentOfPeakViewRowIndex;
			previousAscentOfPeakViewRowIndex	= ascentOfPeakViewRowIndices.at(ascentOfPeakViewRowIndices.indexOf(currentViewRowIndex) - 1);
		}
		if (currentViewRowIndex < maxAscentOfPeakViewRowIndex) {
			nextAscentOfPeakViewRowIndex		= ascentOfPeakViewRowIndices.at(ascentOfPeakViewRowIndices.indexOf(currentViewRowIndex) + 1);
			lastAscentOfPeakViewRowIndex		= maxAscentOfPeakViewRowIndex;
		}
	}
}

void AscentViewer::updateAscentNavigationButtonsEnabled()
{
	firstAscentButton			->setEnabled(firstAscentViewRowIndex			>= 0);
	previousAscentButton		->setEnabled(previousAscentViewRowIndex			>= 0);
	nextAscentButton			->setEnabled(nextAscentViewRowIndex				>= 0);
	lastAscentButton			->setEnabled(lastAscentViewRowIndex				>= 0);
	
	firstAscentOfPeakButton		->setEnabled(firstAscentOfPeakViewRowIndex		>= 0);
	previousAscentOfPeakButton	->setEnabled(previousAscentOfPeakViewRowIndex	>= 0);
	nextAscentOfPeakButton		->setEnabled(nextAscentOfPeakViewRowIndex		>= 0);
	lastAscentOfPeakButton		->setEnabled(lastAscentOfPeakViewRowIndex		>= 0);
}

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



void AscentViewer::changeToPhoto(int photoIndex)
{
	savePhotoDescription();
	
	currentPhotoIndex = photoIndex;
	updatePhoto();
	updatePhotoButtonsEnabled();
}

void AscentViewer::updatePhoto()
{
	photoDescriptionLabel	->setText(QString());
	photoDescriptionLineEdit->setText(QString());
	photoDescriptionLabel	->setVisible(false);
	photoDescriptionLineEdit->setVisible(false);
	
	updatePhotoIndexLabel();
	
	if (currentPhotoIndex < 0 || photos.isEmpty()) {
		imageDisplay->setVisible(false);
		return;
	}
	
	QString filepath = photos.at(currentPhotoIndex).filepath;
	QImageReader reader(filepath);
	reader.setAutoTransform(true);
	const QImage newImage = reader.read();
	if (newImage.isNull()) {
		qDebug() << "Error reading" << filepath << reader.errorString();
		imageDisplay->setVisible(false);
		
		QString title = tr("File error");
		QString message = tr("Photo could not be loaded:")
				+ "\n" + filepath
				+ "\n\n" + tr("Do you want to remove it from this ascent?");
		QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		QMessageBox::StandardButton result = QMessageBox::warning(this, title, message, buttons);
		
		if (result == QMessageBox::Yes) {
			removeCurrentPhoto();
			updatePhoto();
		}
		return;
	}
	
	image = newImage;
	if (image.colorSpace().isValid()) image.convertToColorSpace(QColorSpace::SRgb);
	imageDisplay->setPixmap(QPixmap::fromImage(image));
	imageDisplay->setVisible(true);
	
	photoDescriptionLabel	->setText(photos.at(currentPhotoIndex).description);
	photoDescriptionLineEdit->setText(photos.at(currentPhotoIndex).description);
	photoDescriptionLabel	->setVisible(!photoDescriptionEditable);
	photoDescriptionLineEdit->setVisible(photoDescriptionEditable);
}

void AscentViewer::updatePhotoIndexLabel()
{
	if (currentPhotoIndex < 0) {
		photoIndexLabel->setText(tr("No photos"));
	} else {
		photoIndexLabel->setText(tr("Photo %1 of %2").arg(currentPhotoIndex + 1).arg(photos.size()));
	}
}

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

void AscentViewer::addPhotos()
{
	QStringList filepaths = openFileDialogForPhotosSelection(this);
	if (filepaths.isEmpty()) return;
	
	if (currentPhotoIndex < 0) currentPhotoIndex = -1;
	currentPhotoIndex++;	// Set to index of first inserted photo
	for (int i = 0; i < filepaths.size(); i++) {
		photos.insert(currentPhotoIndex + i, Photo(currentAscentID, ItemID(), -1, filepaths.at(i), QString()));
	}
	
	savePhotosList();
	
	updatePhoto();
}

void AscentViewer::removeCurrentPhoto()
{
	photos.removeAt(currentPhotoIndex);
	savePhotosList();
	
	if (currentPhotoIndex >= photos.size()) currentPhotoIndex = photos.size() - 1;
	updatePhotoButtonsEnabled();
	updatePhoto();
}

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

void AscentViewer::savePhotosList()
{
	for (int i = 0; i < photos.size(); i++) {
		photos[0].sortIndex = i;
	}
	db->photosTable->updateRows(this, currentAscentID.forceValid(), photos);
}



// EDITING DESCRIPTION

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

void AscentViewer::handle_firstAscent()
{
	changeToAscent(firstAscentViewRowIndex);
}

void AscentViewer::handle_previousAscent()
{
	changeToAscent(previousAscentViewRowIndex);
}

void AscentViewer::handle_nextAscent()
{
	changeToAscent(nextAscentViewRowIndex);
}

void AscentViewer::handle_lastAscent()
{
	changeToAscent(lastAscentViewRowIndex);
}

void AscentViewer::handle_firstAscentOfPeak()
{
	changeToAscent(firstAscentOfPeakViewRowIndex);
}

void AscentViewer::handle_previousAscentOfPeak()
{
	changeToAscent(previousAscentOfPeakViewRowIndex);
}

void AscentViewer::handle_nextAscentOfPeak()
{
	changeToAscent(nextAscentOfPeakViewRowIndex);
}

void AscentViewer::handle_lastAscentOfPeak()
{
	changeToAscent(lastAscentOfPeakViewRowIndex);
}


// PHOTO NAVIGATION

void AscentViewer::handle_firstPhoto()
{
	changeToPhoto(0);
}

void AscentViewer::handle_previousPhoto()
{
	changeToPhoto(currentPhotoIndex - 1);
}

void AscentViewer::handle_nextPhoto()
{
	changeToPhoto(currentPhotoIndex + 1);
}

void AscentViewer::handle_lastPhoto()
{
	changeToPhoto(photos.size() - 1);
}


// CHANGING PHOTOS

void AscentViewer::handle_movePhotoLeft()
{
	moveCurrentPhoto(true);
}

void AscentViewer::handle_movePhotoRight()
{
	moveCurrentPhoto(false);
}

void AscentViewer::handle_addPhotos()
{
	addPhotos();
}

void AscentViewer::handle_removePhoto()
{
	removeCurrentPhoto();
}


// RIGHT CLICK

void AscentViewer::handle_rightClickOnAscentInfo(QPoint pos)
{
	popupInfoContextMenu(ascentInfoBox->mapToGlobal(pos));
}

void AscentViewer::handle_rightClickOnPeakInfo(QPoint pos)
{
	popupInfoContextMenu(peakInfoBox->mapToGlobal(pos));
}

void AscentViewer::handle_rightClickOnTripInfo(QPoint pos)
{
	popupInfoContextMenu(tripInfoBox->mapToGlobal(pos));
}


// EDIT ACTIONS

void AscentViewer::handle_descriptionEditableChanged()
{
	if (descriptionEditable) saveDescription();
	
	bool descriptionEditable = editDescriptionButton->isChecked();

	descriptionTextBrowser->setReadOnly(!descriptionEditable);
}

void AscentViewer::handle_photoDescriptionEditableChanged()
{
	if (photoDescriptionEditable) {
		savePhotoDescription();
		photoDescriptionLabel->setText(photoDescriptionLineEdit->text());
	} else {
		photoDescriptionLineEdit->setText(photoDescriptionLabel->text());
	}
	
	bool photoDescriptionEditable = editPhotoDescriptionButton->isChecked();

	photoDescriptionLabel	->setVisible(!photoDescriptionEditable);
	photoDescriptionLineEdit->setVisible(photoDescriptionEditable);
}


void AscentViewer::handle_editAscent()
{
	int oldBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	openEditAscentDialogAndStore(this, db, oldBufferRowIndex);
	handleChangesToUnderlyingData(oldBufferRowIndex);
}

void AscentViewer::handle_editPeak()
{
	int oldAscentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ValidItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(oldAscentBufferRowIndex).toInt();
	int peakBufferRowIndex = db->peaksTable->getBufferIndexForPrimaryKey(peakID);
	openEditPeakDialogAndStore(this, db, peakBufferRowIndex);
	handleChangesToUnderlyingData(oldAscentBufferRowIndex);
}

void AscentViewer::handle_editTrip()
{
	int oldAscentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(currentViewRowIndex);
	ValidItemID tripID = db->ascentsTable->tripIDColumn->getValueFor(oldAscentBufferRowIndex).toInt();
	int tripBufferRowIndex = db->tripsTable->getBufferIndexForPrimaryKey(tripID);
	openEditTripDialogAndStore(this, db, tripBufferRowIndex);
	handleChangesToUnderlyingData(oldAscentBufferRowIndex);
}



void AscentViewer::popupInfoContextMenu(QPoint pos)
{
	ItemID peakID = db->ascentsTable->peakIDColumn->getValueFor(currentAscentID.forceValid()).toInt();
	editPeakAction->setEnabled(peakID.isValid());
	ItemID tripID = db->ascentsTable->tripIDColumn->getValueFor(currentAscentID.forceValid()).toInt();
	editTripAction->setEnabled(tripID.isValid());
	
	infoContextMenu.popup(pos);
}

void AscentViewer::handleChangesToUnderlyingData(int currentBufferRowIndex)
{
	// Filtering and sorting may have changed, update view row index
	int newViewRowIndex = compAscents->findCurrentViewRowIndex(currentBufferRowIndex);
	
	if (newViewRowIndex >= 0) {	// Current ascent still in table
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
	if (newViewRowIndex >= numberOfVisibleRows) newViewRowIndex = numberOfVisibleRows - 1;
	changeToAscent(newViewRowIndex);
}



// EXIT BEHAVIOUR

void AscentViewer::reject()
{
	saveDescription();
	savePhotoDescription();
	saveDialogGeometry(this, mainWindow, &Settings::ascentViewer_geometry);
	saveSplitterSizes();
	QDialog::reject();
}

void AscentViewer::saveSplitterSizes()
{
	QList<int> splitterSizes = centralSplitter->sizes();
	QStringList stringList;
	for (int size : splitterSizes) {
		stringList.append(QString::number(size));
	}
	Settings::ascentViewer_splitterSizes.set(stringList);
}


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
