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

/**
 * @file settings_window.h
 * 
 * This file declares the SettingsWindow class.
 */

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include "src/settings/settings.h"
#include "ui_settings_window.h"



/**
 * Control class for the settings window.
 */
class SettingsWindow : public QDialog, public Ui_SettingsWindow, public Settings
{
	Q_OBJECT
	
	/** The parent window. */
	QWidget* parent;
	
	/** Lists of avalable languages, as codes and native names. */
	QPair<QStringList, QStringList> languages;
	
public:
	SettingsWindow(QWidget* parent);
	
private:
	void loadSettings();
	void saveSettings();
	void loadDefaults();
	void updateEnabled();
	
private slots:
	void handle_rememberWindowPositionsCheckboxChanged();
	
	void handle_ascentDateCheckboxChanged();
	void handle_ascentTimeCheckboxChanged();
	void handle_ascentElevationGainCheckboxChanged();
	void handle_peakHeightCheckboxChanged();
	
	void handle_save();
	void handle_apply();
	void handle_cancel();
	void handle_loadDefaults();
	
private:
	void reject() override;
};



#endif // SETTINGS_WINDOW_H
