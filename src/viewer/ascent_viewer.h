#ifndef ASCENT_VIEWER_H
#define ASCENT_VIEWER_H

#include "src/viewer/image_display.h"
#include "ui_ascent_viewer.h"



class AscentViewer : public QDialog, public Ui_AscentViewer {
	Q_OBJECT
	
	ImageDisplay* imageDisplay;
	QImage image;
	
public:
	AscentViewer(QWidget* parent);
	virtual ~AscentViewer();
	
	inline void displayTestImage();
};



#endif // ASCENT_VIEWER_H
