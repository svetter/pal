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
 * @file project_settings.h
 * 
 * This file declares the GenericProjectSetting, ProjectSetting and ProjectMultiSetting classes
 * and defines the ProjectSettings class.
 */

#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include "src/db/tables/settings_table.h"

#include <QString>
#include <QVariant>
#include <QDate>



/**
 * A class representing a project setting without a type.
 * 
 * This class exists to work around template quirks.
 */
class GenericProjectSetting {
public:
	/** The project settings table. */
	SettingsTable* const table;
	/** The key string which is used to identify this setting. */
	const QString key;
	/** The default value for the setting. */
	const QVariant defaultValue;
	
	GenericProjectSetting(SettingsTable* table, const QString& key, QVariant defaultValue);
	
	bool isPresent(QWidget* parent = nullptr) const;
	QVariant getAsQVariant(QWidget* parent = nullptr) const;
	QVariant getDefaultAsQVariant() const;
	
	void set(QWidget* parent, QVariant value);
	void clear(QWidget* parent);
	void remove(QWidget* parent);
};



/**
 * A class representing a project setting.
 */
template<typename T>
class ProjectSetting : public GenericProjectSetting {
public:
	ProjectSetting(SettingsTable* table, const QString& key, QVariant defaultValue = T());
	
	T get(QWidget* parent = nullptr);
	T getDefault() const;
};

// List used types as compiler hints
template class ProjectSetting<bool>;
template class ProjectSetting<int>;
template class ProjectSetting<QString>;
template class ProjectSetting<QDate>;



template<typename T>
class ProjectMultiSetting
{
	/** Dynamically grown list of all settings in this group, mapped to their sub-keys. */
	QMap<QString, ProjectSetting<T>*> settings;
	
	/** The project settings table. */
	SettingsTable* const table;
	/** The part of the key which all settings share. */
	const QString baseKey;
	/** The default value of all the settings. */
	const QVariant defaultValue;
	
public:
	ProjectMultiSetting(SettingsTable* table, const QString baseKey, QVariant defaultValue = T());
	
	bool anyPresent() const;
	bool nonePresent() const;
	bool allPresent(QSet<QString> subKeys);
	
	QMap<QString, T> get(const QSet<QString>& subKeys);
	T getDefault() const;
	
	void set(QWidget* parent, const QMap<QString, T>& subKeyValueMap);
	void clear(QWidget* parent, SettingsTable* settingsTable) const;
	
private:
	void createSettingIfMissing(const QString& subKey);
};



/**
 * A class for managing all project settings.
 */
class ProjectSettings {
public:
	/** The version string of the application with which the database was last written. */
	ProjectSetting<QString>	databaseVersion;
	
	/** The default hiker setting. */
	ProjectSetting<int>		defaultHiker;
	
	// Implicit settings
	/** The primary date filter setting. Represents a filter for only this date when no max setting is present, and as a minimum date otherwise. */
	ProjectSetting<QDate>	ascentFilterDate;
	/** The maximum date filter setting. */
	ProjectSetting<QDate>	ascentFilterMaxDate;
	/** The primary peak height filter setting. Represents a filter for only this 1000s class of height when no max setting is present, and as a minimum height otherwise. */
	ProjectSetting<int>		ascentFilterPeakHeight;
	/** The maximum peak height filter setting. */
	ProjectSetting<int>		ascentFilterMaxPeakHeight;
	/** The volcano filter setting. */
	ProjectSetting<bool>	ascentFilterVolcano;
	/** The range filter setting. */
	ProjectSetting<int>		ascentFilterRange;
	/** The hike kind filter setting. */
	ProjectSetting<int>		ascentFilterHikeKind;
	/** The difficulty system filter setting. Only valid when the difficulty grade setting is also present. */
	ProjectSetting<int>		ascentFilterDifficultySystem;
	/** The difficulty grade filter setting. */
	ProjectSetting<int>		ascentFilterDifficultyGrade;
	/** The hiker filter setting. */
	ProjectSetting<int>		ascentFilterHiker;
	
	
	
	/**
	 * Creates a new ProjectSettings object.
	 * 
	 * Creates all project setting objects dynamically. The ProjectSettings object as well as all
	 * settings objects do not neet to be changed, destroyed or recreated when a project is closed
	 * or opened.
	 */
	inline ProjectSettings(SettingsTable* table) :
			//												type				key																default value
			databaseVersion					(ProjectSetting<QString>	(table,	"databaseVersion")),
			defaultHiker					(ProjectSetting<int>		(table,	"defaultHiker")),
			// Implicit settings
			ascentFilterDate				(ProjectSetting<QDate>		(table,	"implicit/mainWindow/filters/ascentsTable/date")),
			ascentFilterMaxDate				(ProjectSetting<QDate>		(table,	"implicit/mainWindow/filters/ascentsTable/dateMax")),
			ascentFilterPeakHeight			(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/peakHeight")),
			ascentFilterMaxPeakHeight		(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/peakHeightMax")),
			ascentFilterVolcano				(ProjectSetting<bool>		(table,	"implicit/mainWindow/filters/ascentsTable/volcano")),
			ascentFilterRange				(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/range")),
			ascentFilterHikeKind			(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/hikeKind")),
			ascentFilterDifficultySystem	(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/difficultySystem")),
			ascentFilterDifficultyGrade		(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/difficultyGrade")),
			ascentFilterHiker				(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/hiker"))
	{}
};



#endif // PROJECT_SETTINGS_H
