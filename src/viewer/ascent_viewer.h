#ifndef ASCENT_VIEWER_H
#define ASCENT_VIEWER_H

#include "src/main/main_window.h"
#include "src/viewer/image_display.h"
#include "ui_ascent_viewer.h"



class AscentViewer : public QDialog, public Ui_AscentViewer {
	Q_OBJECT
	
	MainWindow* mainWindow;
	Database* db;
	CompositeAscentsTable* compAscents;
	int viewRowIndex;
	
	ImageDisplay* imageDisplay;
	QImage image;
	
public:
	AscentViewer(MainWindow* parent, Database* db, CompositeAscentsTable* compAscents, int viewRowIndex);
	virtual ~AscentViewer();
	
	inline void displayTestImage();
};



#endif // ASCENT_VIEWER_H
