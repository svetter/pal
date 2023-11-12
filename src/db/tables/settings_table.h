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
 * @file settings_table.h
 * 
 * This file declares the SettingsTable class.
 */

#ifndef SETTINGS_TABLE_H
#define SETTINGS_TABLE_H

#include "src/db/table.h"

#include <QWidget>
#include <QVariant>

class GenericProjectSetting;



/**
 * A class for accessing and manipulating the project settings table in the database.
 */
class SettingsTable : public Table {
	/** The primary key column. */
	Column* primaryKeyColumn;
	/** The column for key strings which identify settings. */
	Column* settingKeyColumn;
	/** The column for setting values, encoded as strings. */
	Column* settingValueColumn;
	
public:
	SettingsTable();
	
	bool settingIsPresent(const GenericProjectSetting* setting, QWidget* parent = nullptr);
	QVariant getSetting(const GenericProjectSetting* setting, QWidget* parent = nullptr);
	
	void setSetting(QWidget* parent, const GenericProjectSetting* setting, QVariant value);
	void clearSetting(QWidget* parent, const GenericProjectSetting* setting);
	void removeSetting(QWidget* parent, const GenericProjectSetting* setting);
	
private:
	ItemID findSettingID(const GenericProjectSetting* setting, QWidget* parent = nullptr);
};



#endif // SETTINGS_TABLE_H
