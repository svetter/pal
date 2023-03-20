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
	ItemID currentAscentID;
	
	int firstAscentViewRowIndex;
	int previousAscentViewRowIndex;
	int nextAscentViewRowIndex;
	int lastAscentViewRowIndex;
	
	int firstAscentOfPeakViewRowIndex;
	int previousAscentOfPeakViewRowIndex;
	int nextAscentOfPeakViewRowIndex;
	int lastAscentOfPeakViewRowIndex;
	
	QList<Photo> photos;
	int currentPhotoIndex;
	
	ImageDisplay* imageDisplay;
	QImage image;
	
	QMenu infoContextMenu;
	QMenu photoDescriptionContextMenu;
	
public:
	AscentViewer(MainWindow* parent, Database* db, const ItemTypesHandler* typesHandler, int viewRowIndex);
	virtual ~AscentViewer();
	
private:
	// Initial setup
	void additionalUISetup();
	void connectUI();
	void setupContextMenus();
	void setupShortcuts();
	
	// Ascent change
	void changeToAscent(int viewRowIndex);
	void resetInfoLabels();
	void updateAscentInfo();
	void updateAscentNavigationTargets();
	void updateAscentNavigationButtonsEnabled();
	void setupPhotos();
	
	// Photo change
	void changeToPhoto(int photoIndex);
	void updatePhoto();
	void updatePhotoIndexLabel();
	void updatePhotoButtonsEnabled();
	
	// Editing photos
	void moveCurrentPhoto(bool moveLeftNotRight);
	void addPhotos();
	void removeCurrentPhoto();
	void savePhotosList();
	
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
	void handle_addPhotos();
	void handle_removePhoto();
	// Right click
	void handle_rightClickOnAscentInfo(QPoint pos);
	void handle_rightClickOnPeakInfo(QPoint pos);
	void handle_rightClickOnTripInfo(QPoint pos);
	void handle_rightClickOnPhotoDescription(QPoint pos);
	// Edit actions
	void handle_editAscent();
	void handle_editPeak();
	void handle_editTrip();
	void handle_editPhotoDescription();
};



#endif // ASCENT_VIEWER_H
