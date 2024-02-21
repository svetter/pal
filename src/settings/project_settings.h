/*
 * Copyright 2023-2024 Simon Vetter
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
	SettingsTable& table;
	/** The key string which is used to identify this setting. */
	const QString key;
	/** The default value for the setting. */
	const QVariant defaultValue;
	
	GenericProjectSetting(SettingsTable& table, const QString& key, QVariant defaultValue);
	
	bool present(QWidget* parent = nullptr) const;
	QVariant getAsQVariant(QWidget* parent = nullptr) const;
	QVariant getDefaultAsQVariant() const;
	
	void set(QWidget* parent, QVariant value) const;
	void clear(QWidget* parent) const;
	void remove(QWidget* parent) const;
};



/**
 * A class representing a project setting.
 */
template<typename T>
class ProjectSetting : public GenericProjectSetting {
public:
	ProjectSetting(SettingsTable& table, const QString& key, QVariant defaultValue = T());
	
	T get(QWidget* parent = nullptr) const;
	T getDefault() const;
};



template<typename T>
class ProjectMultiSetting
{
	/** Dynamically grown list of all settings in this group, mapped to their sub-keys. */
	QMap<QString, ProjectSetting<T>*> settings;
	
	/** The project settings table. */
	SettingsTable& table;
	/** The part of the key which all settings share. */
	const QString baseKey;
	/** The default value of all the settings. */
	const QVariant defaultValue;
	
public:
	ProjectMultiSetting(SettingsTable& table, const QString baseKey, QVariant defaultValue = T());
	~ProjectMultiSetting();
	
	bool anyPresent(const QSet<QString>& subKeys);
	bool nonePresent(const QSet<QString>& subKeys);
	bool allPresent(const QSet<QString>& subKeys);
	
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
	ProjectSetting<QString>		databaseVersion;
	
	// === EXPLICIT PROJECT SETTINGS ===
	
	/** The default hiker setting. */
	ProjectSetting<int>			defaultHiker;
	
	
	// === IMPLICIT SETTINGS ===
	
	// View state
	/** Remembered index of open tab in the main window. */
	ProjectSetting<int>			mainWindow_currentTabIndex;
	/** Remember filters in the main window as being visible. */
	ProjectSetting<bool>		mainWindow_showFilterBar;
	
	// Column widths
	/** Remembered column widths of the ascents table in the main window. */
	ProjectMultiSetting<int>	columnWidths_ascentsTable;
	/** Remembered column widths of the peaks table in the main window. */
	ProjectMultiSetting<int>	columnWidths_peaksTable;
	/** Remembered column widths of the trips table in the main window. */
	ProjectMultiSetting<int>	columnWidths_tripsTable;
	/** Remembered column widths of the hikers table in the main window. */
	ProjectMultiSetting<int>	columnWidths_hikersTable;
	/** Remembered column widths of the regions table in the main window. */
	ProjectMultiSetting<int>	columnWidths_regionsTable;
	/** Remembered column widths of the ranges table in the main window. */
	ProjectMultiSetting<int>	columnWidths_rangesTable;
	/** Remembered column widths of the countries table in the main window. */
	ProjectMultiSetting<int>	columnWidths_countriesTable;
	
	// Column order
	/** Remembered column order of the ascents table in the main window. */
	ProjectMultiSetting<int>	columnOrder_ascentsTable;
	/** Remembered column order of the peaks table in the main window. */
	ProjectMultiSetting<int>	columnOrder_peaksTable;
	/** Remembered column order of the trips table in the main window. */
	ProjectMultiSetting<int>	columnOrder_tripsTable;
	/** Remembered column order of the hikers table in the main window. */
	ProjectMultiSetting<int>	columnOrder_hikersTable;
	/** Remembered column order of the regions table in the main window. */
	ProjectMultiSetting<int>	columnOrder_regionsTable;
	/** Remembered column order of the ranges table in the main window. */
	ProjectMultiSetting<int>	columnOrder_rangesTable;
	/** Remembered column order of the countries table in the main window. */
	ProjectMultiSetting<int>	columnOrder_countriesTable;
	
	// Hidden columns
	/** Remembered column hidden states of the ascents table in the main window. */
	ProjectMultiSetting<bool>	hiddenColumns_ascentsTable;
	/** Remembered column hidden states of the peaks table in the main window. */
	ProjectMultiSetting<bool>	hiddenColumns_peaksTable;
	/** Remembered column hidden states of the trips table in the main window. */
	ProjectMultiSetting<bool>	hiddenColumns_tripsTable;
	/** Remembered column hidden states of the hikers table in the main window. */
	ProjectMultiSetting<bool>	hiddenColumns_hikersTable;
	/** Remembered column hidden states of the regions table in the main window. */
	ProjectMultiSetting<bool>	hiddenColumns_regionsTable;
	/** Remembered column hidden states of the ranges table in the main window. */
	ProjectMultiSetting<bool>	hiddenColumns_rangesTable;
	/** Remembered column hidden states of the countries table in the main window. */
	ProjectMultiSetting<bool>	hiddenColumns_countriesTable;
	
	// Sorting
	/** Remembered sorting of the ascents table in the main window. */
	ProjectSetting<QString>		sorting_ascentsTable;
	/** Remembered sorting of the peaks table in the main window. */
	ProjectSetting<QString>		sorting_peaksTable;
	/** Remembered sorting of the trips table in the main window. */
	ProjectSetting<QString>		sorting_tripsTable;
	/** Remembered sorting of the hikers table in the main window. */
	ProjectSetting<QString>		sorting_hikersTable;
	/** Remembered sorting of the regions table in the main window. */
	ProjectSetting<QString>		sorting_regionsTable;
	/** Remembered sorting of the ranges table in the main window. */
	ProjectSetting<QString>		sorting_rangesTable;
	/** Remembered sorting of the countries table in the main window. */
	ProjectSetting<QString>		sorting_countriesTable;
	
	// Ascent table filters
	/** The primary date filter setting. Represents a filter for only this date when no max setting is present, and as a minimum date otherwise. */
	ProjectSetting<QDate>		ascentFilters_date;
	/** The maximum date filter setting. */
	ProjectSetting<QDate>		ascentFilters_maxDate;
	/** The primary peak height filter setting. Represents a filter for only this 1000s class of height when no max setting is present, and as a minimum height otherwise. */
	ProjectSetting<int>			ascentFilters_peakHeight;
	/** The maximum peak height filter setting. */
	ProjectSetting<int>			ascentFilters_maxPeakHeight;
	/** The volcano filter setting. */
	ProjectSetting<bool>		ascentFilters_volcano;
	/** The range filter setting. */
	ProjectSetting<int>			ascentFilters_range;
	/** The hike kind filter setting. */
	ProjectSetting<int>			ascentFilters_hikeKind;
	/** The difficulty system filter setting. Only valid when the difficulty grade setting is also present. */
	ProjectSetting<int>			ascentFilters_difficultySystem;
	/** The difficulty grade filter setting. */
	ProjectSetting<int>			ascentFilters_difficultyGrade;
	/** The hiker filter setting. */
	ProjectSetting<int>			ascentFilters_hiker;
	
	
	
	/**
	 * Creates a new ProjectSettings object.
	 * 
	 * Creates all project setting objects dynamically. The ProjectSettings object as well as all
	 * settings objects do not neet to be changed, destroyed or recreated when a project is closed
	 * or opened.
	 */
	inline ProjectSettings(SettingsTable& table) :
		//															key																		default value
		databaseVersion					(ProjectSetting<QString>	(table,	"databaseVersion")),
		
		// === EXPLICIT PROJECT SETTINGS ===
		
		defaultHiker					(ProjectSetting<int>		(table,	"defaultHiker")),
		
		
		// === IMPLICIT SETTINGS ===
		
		// View state
		mainWindow_currentTabIndex		(ProjectSetting<int>		(table,	"implicit/mainWindow/currentTabIndex",							0)),
		mainWindow_showFilterBar		(ProjectSetting<bool>		(table,	"implicit/mainWindow/showFilters",								true)),
		
		// Column widths
		columnWidths_ascentsTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnWidths/ascentsTable",				100)),
		columnWidths_peaksTable			(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnWidths/peaksTable",					100)),
		columnWidths_tripsTable			(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnWidths/tripsTable",					100)),
		columnWidths_hikersTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnWidths/hikersTable",					100)),
		columnWidths_regionsTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnWidths/regionsTable",				100)),
		columnWidths_rangesTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnWidths/rangesTable",					100)),
		columnWidths_countriesTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnWidths/countriesTable",				100)),
		
		// Column widths
		columnOrder_ascentsTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnOrder/ascentsTable",					-1)),
		columnOrder_peaksTable			(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnOrder/peaksTable",					-1)),
		columnOrder_tripsTable			(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnOrder/tripsTable",					-1)),
		columnOrder_hikersTable			(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnOrder/hikersTable",					-1)),
		columnOrder_regionsTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnOrder/regionsTable",					-1)),
		columnOrder_rangesTable			(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnOrder/rangesTable",					-1)),
		columnOrder_countriesTable		(ProjectMultiSetting<int>	(table,	"implicit/mainWindow/columnOrder/countriesTable",				-1)),
		
		// Column widths
		hiddenColumns_ascentsTable		(ProjectMultiSetting<bool>	(table,	"implicit/mainWindow/hiddenColumns/ascentsTable",				false)),
		hiddenColumns_peaksTable		(ProjectMultiSetting<bool>	(table,	"implicit/mainWindow/hiddenColumns/peaksTable",					false)),
		hiddenColumns_tripsTable		(ProjectMultiSetting<bool>	(table,	"implicit/mainWindow/hiddenColumns/tripsTable",					false)),
		hiddenColumns_hikersTable		(ProjectMultiSetting<bool>	(table,	"implicit/mainWindow/hiddenColumns/hikersTable",				false)),
		hiddenColumns_regionsTable		(ProjectMultiSetting<bool>	(table,	"implicit/mainWindow/hiddenColumns/regionsTable",				false)),
		hiddenColumns_rangesTable		(ProjectMultiSetting<bool>	(table,	"implicit/mainWindow/hiddenColumns/rangesTable",				false)),
		hiddenColumns_countriesTable	(ProjectMultiSetting<bool>	(table,	"implicit/mainWindow/hiddenColumns/countriesTable",				false)),
		
		// Sorting
		sorting_ascentsTable			(ProjectSetting<QString>	(table,	"implicit/mainWindow/sorting/ascentsTable")),
		sorting_peaksTable				(ProjectSetting<QString>	(table,	"implicit/mainWindow/sorting/peaksTable")),
		sorting_tripsTable				(ProjectSetting<QString>	(table,	"implicit/mainWindow/sorting/tripsTable")),
		sorting_hikersTable				(ProjectSetting<QString>	(table,	"implicit/mainWindow/sorting/hikersTable")),
		sorting_regionsTable			(ProjectSetting<QString>	(table,	"implicit/mainWindow/sorting/regionsTable")),
		sorting_rangesTable				(ProjectSetting<QString>	(table,	"implicit/mainWindow/sorting/rangesTable")),
		sorting_countriesTable			(ProjectSetting<QString>	(table,	"implicit/mainWindow/sorting/countriesTable")),
		
		// Ascent table filters
		ascentFilters_date				(ProjectSetting<QDate>		(table,	"implicit/mainWindow/filters/ascentsTable/date")),
		ascentFilters_maxDate			(ProjectSetting<QDate>		(table,	"implicit/mainWindow/filters/ascentsTable/dateMax")),
		ascentFilters_peakHeight		(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/peakHeight")),
		ascentFilters_maxPeakHeight		(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/peakHeightMax")),
		ascentFilters_volcano			(ProjectSetting<bool>		(table,	"implicit/mainWindow/filters/ascentsTable/volcano")),
		ascentFilters_range				(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/range")),
		ascentFilters_hikeKind			(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/hikeKind")),
		ascentFilters_difficultySystem	(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/difficultySystem")),
		ascentFilters_difficultyGrade	(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/difficultyGrade")),
		ascentFilters_hiker				(ProjectSetting<int>		(table,	"implicit/mainWindow/filters/ascentsTable/hiker"))
	{}
};



#endif // PROJECT_SETTINGS_H
