#ifndef PROJECT_SETTINGS_WINDOW_H
#define PROJECT_SETTINGS_WINDOW_H

#include "src/db/database.h"
#include "ui_project_settings_window.h"

#include <QDialog>



class ProjectSettingsWindow : public QDialog, public Ui_ProjectSettingsWindow
{
	Q_OBJECT
	
	Database* db;
	
public:
	ProjectSettingsWindow(QWidget* parent, Database* db);
	
private:
	void loadSettings();
	void saveSettings();
	void updateEnabled();
	
private slots:	
	void handle_newHiker();
	void handle_photosBasePathCheckboxChanged();
	void handle_photosBasePathBrowse();
	
	void handle_save();
	void handle_apply();
	void handle_cancel();
	
private:
	void reject() override;
};



#endif // PROJECT_SETTINGS_WINDOW_H
