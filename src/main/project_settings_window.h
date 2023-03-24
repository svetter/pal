#ifndef PROJECT_SETTINGS_WINDOW_H
#define PROJECT_SETTINGS_WINDOW_H

#include "src/db/database.h"
#include "ui_project_settings_window.h"

#include <QDialog>



class ProjectSettingsWindow : public QDialog, public Ui_ProjectSettingsWindow
{
	Q_OBJECT
	
	QWidget* parent;
	
	Database* db;
	bool firstOpen;
	
	QList<ValidItemID> selectableHikerIDs;
	
public:
	ProjectSettingsWindow(QWidget* parent, Database* db, bool firstOpen = false);
	
private:
	void loadSettings();
	void saveSettings();
	
private slots:	
	void handle_newHiker();
	
	void handle_save();
	void handle_apply();
	void handle_cancel();
	
private:
	void reject() override;
};



#endif // PROJECT_SETTINGS_WINDOW_H
