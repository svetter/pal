#ifndef ASCENT_VIEWER_H
#define ASCENT_VIEWER_H

#include "src/main/main_window.h"
#include "src/viewer/image_display.h"
#include "ui_ascent_viewer.h"



class AscentViewer : public QDialog, public Ui_AscentViewer {
	Q_OBJECT
	
	MainWindow* const mainWindow;
	Database* const db;
	CompositeAscentsTable* const compAscents;
	CompositePeaksTable* const compPeaks;
	CompositeTripsTable* const compTrips;
	
	int currentViewRowIndex;
	
	int firstAscentViewRowIndex;
	int previousAscentViewRowIndex;
	int nextAscentViewRowIndex;
	int lastAscentViewRowIndex;
	
	int firstAscentOfPeakViewRowIndex;
	int previousAscentOfPeakViewRowIndex;
	int nextAscentOfPeakViewRowIndex;
	int lastAscentOfPeakViewRowIndex;
	
	ImageDisplay* imageDisplay;
	QImage image;
	
public:
	AscentViewer(MainWindow* parent, Database* db, const ItemTypesHandler* typesHandler, int viewRowIndex);
	virtual ~AscentViewer();
	
private:
	// Initial setup
	void additionalUISetup();
	void connectUI();
	void initContextMenusAndShortcuts();
	
	// Ascent change
	void resetInfoLabels();
	void insertInfoIntoUI(int viewRowIndex);
	void updateNavigationTargets(int viewRowIndex);
	void updateNavigationButtonsEnabled();
	void changeToEntry(int viewRowIndex);
	
private slots:
	// Ascent navigation
	void handle_firstAscent();
	void handle_previousAscent();
	void handle_nextAscent();
	void handle_lastAscent();
	void handle_firstAscentOfPeak();
	void handle_previousAscentOfPeak();
	void handle_nextAscentOfPeak();
	void handle_lastAscentOfPeak();
	// Photo navigation
	void handle_firstPhoto();
	void handle_previousPhoto();
	void handle_nextPhoto();
	void handle_lastPhoto();
	// Changing photos
	void handle_movePhotoLeft();
	void handle_movePhotoRight();
	void handle_addPhoto();
	void handle_removePhoto();
	// Right click
	void handle_rightClickOnTrip(QPoint pos);
	void handle_rightClickOnAscent(QPoint pos);
	void handle_rightClickOnPeak(QPoint pos);
	void handle_rightClickOnPhotoDescription(QPoint pos);
	// Edit actions
	void handle_editAscent();
	void handle_editPeak();
	void handle_editPhotoDescription();
	
private:
	void displayTestImage();
};



#endif // ASCENT_VIEWER_H
