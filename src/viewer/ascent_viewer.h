#ifndef ASCENT_VIEWER_H
#define ASCENT_VIEWER_H

#include "qstyle.h"
#include "src/viewer/image_display.h"
#include "ui_ascent_viewer.h"

#include <QDialog>
#include <QMessageBox>
#include <QDir>
#include <QImageReader>
#include <QColorSpace>
#include <QScrollBar>



class AscentViewer : public QDialog, public Ui_AscentViewer {
	Q_OBJECT
	
	ImageDisplay* imageDisplay;
	QImage image;
	
public:
	inline AscentViewer(QWidget* parent) :
			QDialog(parent)
	{
		setupUi(this);
		
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
		
		
		displayTestImage();
	}
	
	inline virtual ~AscentViewer()
	{}
	
	
	
	inline void displayTestImage()
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
};



#endif // ASCENT_VIEWER_H
