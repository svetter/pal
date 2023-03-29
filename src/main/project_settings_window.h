/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

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
