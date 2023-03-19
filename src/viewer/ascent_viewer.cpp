#include "ascent_viewer.h"

#include <QDialog>
#include <QStyle>
#include <QImageReader>
#include <QMessageBox>
#include <QDir>
#include <QColorSpace>



AscentViewer::AscentViewer(MainWindow* parent, Database* db, CompositeAscentsTable* compAscents, int viewRowIndex) :
		QDialog(parent),
		mainWindow(parent),
		db(db),
		compAscents(compAscents),
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

void AscentViewer::insertInfoIntoUI(int viewRowIndex)
{
	// TODO
	qDebug() << "UNIMPLEMENTED: AscentViewer::insertInfoIntoUI()" << viewRowIndex;
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
