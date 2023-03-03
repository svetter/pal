#ifndef PROJECT_SETTINGS_WINDOW_H
#define PROJECT_SETTINGS_WINDOW_H

#include "src/db/database.h"
#include "ui_relocate_photos_window.h"

#include <QDialog>



class RelocatePhotosWindow : public QDialog, public Ui_RelocatePhotosWindow
{
	Q_OBJECT
	
	Database* db;
	
	bool running;
	
public:
	RelocatePhotosWindow(QWidget* parent, Database* db);
	
private slots:
	void handle_browseOldPath();
	void handle_browseNewPath();
	
	void handle_oldPathChanged();
	void handle_newPathChanged();
	
	void handle_start();
	void handle_abort();
	void handle_close();
	
private:
	void reject() override;
};



#endif // PROJECT_SETTINGS_WINDOW_H
