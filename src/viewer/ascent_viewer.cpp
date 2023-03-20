#include "ascent_viewer.h"

#include <QDialog>
#include <QStyle>
#include <QImageReader>
#include <QMessageBox>
#include <QDir>
#include <QColorSpace>



AscentViewer::AscentViewer(MainWindow* parent, Database* db, const ItemTypesHandler* typesHandler, int viewRowIndex) :
		QDialog(parent),
		mainWindow(parent),
		db(db),
		compAscents((CompositeAscentsTable*) typesHandler->get(Ascent)->compTable),
		compPeaks((CompositePeaksTable*) typesHandler->get(Peak)->compTable),
		compTrips((CompositeTripsTable*) typesHandler->get(Trip)->compTable),
		viewRowIndex(viewRowIndex)
{
	setupUi(this);
	additionalUISetup();
	
	connectUI();
	initContextMenusAndShortcuts();
	
	insertInfoIntoUI(viewRowIndex);
	
	displayTestImage();
}

AscentViewer::~AscentViewer()
{}



// INITIAL SETUP

void AscentViewer::additionalUISetup()
{
	peakVolcanoCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
	ascentTraverseCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);
	
	centralSplitter->setStretchFactor(0, 1);
	centralSplitter->setStretchFactor(1, 2);
	centralSplitter->setSizes({ centralSplitter->size().width() / 2, centralSplitter->size().width() / 2 });
	
	imageDisplay = new ImageDisplay(imageFrame);
	imageDisplay->setText(tr("No photos"));
	imageFrameLayout->addWidget(imageDisplay);
	
	firstPhotoButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastPhotoButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	addPhotoButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	removePhotoButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	
	firstAscentButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastAscentButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	firstAscentOfPeakButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	lastAscentOfPeakButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	
	movePhotoLeftButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	movePhotoRightButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
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
	connect(addPhotoButton,				&QToolButton::clicked,	this,	&AscentViewer::handle_addPhoto);
	connect(removePhotoButton,			&QToolButton::clicked,	this,	&AscentViewer::handle_removePhoto);
	// Context menus
	connect(tripInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnTrip);
	connect(ascentInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnAscent);
	connect(peakInfoBox,				&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnPeak);
	connect(photoDescriptionLabel,		&QGroupBox::customContextMenuRequested,	this,	&AscentViewer::handle_rightClickOnPhotoDescription);
}

void AscentViewer::initContextMenusAndShortcuts()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::initContextMenusAndShortcuts()";
}



// ASCENT CHANGE

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

void AscentViewer::insertInfoIntoUI(int viewRowIndex)
{
	resetInfoLabels();
	
	qDebug() << compAscents->name;
	int ascentBufferRowIndex = compAscents->getBufferRowIndexForViewRow(viewRowIndex);
	
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
	ascentPeakOnDayLabel		->setText	(db->ascentsTable->peakOnDayColumn	->getValueAt(ascentBufferRowIndex).toString());
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



// === UI EVENT HANDLERS ===

// ASCENT NAVIGATION

void AscentViewer::handle_firstAscent()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_firstAscent()";
}

void AscentViewer::handle_previousAscent()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_previousAscent()";
}

void AscentViewer::handle_nextAscent()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_nextAscent()";
}

void AscentViewer::handle_lastAscent()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_lastAscent()";
}

void AscentViewer::handle_firstAscentOfPeak()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_firstAscentOfPeak()";
}

void AscentViewer::handle_previousAscentOfPeak()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_previousAscentOfPeak()";
}

void AscentViewer::handle_nextAscentOfPeak()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_nextAscentOfPeak()";
}

void AscentViewer::handle_lastAscentOfPeak()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_lastAscentOfPeak()";
}


// PHOTO NAVIGATION

void AscentViewer::handle_firstPhoto()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_firstPhoto()";
}

void AscentViewer::handle_previousPhoto()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_previousPhoto()";
}

void AscentViewer::handle_nextPhoto()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_nextPhoto()";
}

void AscentViewer::handle_lastPhoto()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_lastPhoto()";
}


// CHANGING PHOTOS

void AscentViewer::handle_movePhotoLeft()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_movePhotoLeft()";
}

void AscentViewer::handle_movePhotoRight()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_movePhotoRight()";
}

void AscentViewer::handle_addPhoto()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_addPhoto()";
}

void AscentViewer::handle_removePhoto()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_removePhoto()";
}


// RIGHT CLICK

void AscentViewer::handle_rightClickOnTrip(QPoint pos)
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_rightClickOnTrip()" << pos;
}

void AscentViewer::handle_rightClickOnAscent(QPoint pos)
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_rightClickOnAscent()" << pos;
}

void AscentViewer::handle_rightClickOnPeak(QPoint pos)
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_rightClickOnPeak()" << pos;
}

void AscentViewer::handle_rightClickOnPhotoDescription(QPoint pos)
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_rightClickOnPhotoDescription()" << pos;
}


// EDIT ACTIONS

void AscentViewer::handle_editAscent()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_editAscent()";
}

void AscentViewer::handle_editPeak()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_editPeak()";
}

void AscentViewer::handle_editPhotoDescription()
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::handle_editPhotoDescription()";
}



void AscentViewer::displayTestImage()
{
	QString filepath = "../testimage.jpg";
	QImageReader reader(filepath);
	reader.setAutoTransform(true);
	const QImage newImage = reader.read();
	if (newImage.isNull()) {
		QString message = tr("Cannot load %1: %2").arg(QDir::toNativeSeparators(filepath), reader.errorString());
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(), message);
	}
	
	image = newImage;
	if (image.colorSpace().isValid())
		image.convertToColorSpace(QColorSpace::SRgb);
	imageDisplay->setPixmap(QPixmap::fromImage(image));
}