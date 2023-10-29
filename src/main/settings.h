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
 * @file settings.h
 * 
 * This file defines the Setting class, declares the Settings class and defines part of it.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDate>
#include <QTime>
#include <QSettings>
#include <QRect>
#include <QWidget>
#include <QDirIterator>



/** PAL uses QSettings with INI format to store general (not project-specific) settings. */
inline QSettings qSettings = QSettings(QSettings::IniFormat, QSettings::UserScope, "PeakAscentLogger", "PeakAscentLogger");



QPair<QStringList, QStringList> getSupportedLanguages();
QString getDefaultLanguage();



/**
 * A class to store a single setting.
 * 
 * @tparam T The type of the setting. Possible types are anything that can be stored in a QVariant.
 */
template<typename T>
class Setting {
	/** The key under which the setting is stored. */
	const QString key;
	/** The default value of the setting. */
	const QVariant defaultValue;
	
public:
	/**
	 * Creates a new setting with the given key and default value.
	 * 
	 * @param key			The key under which the setting will be stored.
	 * @param defaultValue	The default value for the setting.
	 */
	inline Setting(const QString key, QVariant defaultValue = QVariant()) :
			key(key),
			defaultValue(defaultValue)
	{}
	
	/**
	 * Checks whether the setting is present in the settings file.
	 */
	inline bool present() const
	{
		return qSettings.contains(key);
	}
	
	/**
	 * Returns the value of the setting after validating it.
	 * 
	 * If the setting is not present in the settings file or invalid, it is discarded and the
	 * default value is returned and written back to the settings file.
	 * 
	 * @return The value of the setting as it is stored in the settings file after validation.
	 */
	inline T get() const
	{
		if (qSettings.contains(key)) {
			QVariant lookupResult = qSettings.value(key);
			
			// discard if invalid
			if (!lookupResult.canConvert<T>()) {
				clear();
				qDebug() << "Discarded invalid setting" << key << lookupResult;
			}
		}
		
		// Set default if not set
		if (!qSettings.contains(key) && defaultValue.isValid()) {
			qSettings.setValue(key, defaultValue);
		}
		
		return qSettings.value(key).value<T>();
	}
	
	/**
	 * Returns the default value of the setting.
	 * 
	 * @return The default value of the setting.
	 */
	inline T getDefault() const{
		return defaultValue.value<T>();
	}
	
	/**
	 * Sets the value of the setting.
	 * 
	 * @param value The new value for the setting.
	 */
	inline void set(T value) const
	{
		qSettings.setValue(key, QVariant::fromValue(value));
	}
	
	/**
	 * Removes the setting from the settings file.
	 */
	inline void clear() const
	{
		qSettings.remove(key);
	}
};



/**
 * A class to globally store all settings.
 * 
 * This class contains all settings that are not project-specific.
 */
class Settings {
public:
	// === APP VERSION ===

	/** The version of the application with which the settings file was last written. */
	inline static const Setting<QString>	appVersion								= Setting<QString>	("appVersion",									"0.0.0");
	
	
	// === EXPLICIT ===
	
	// Language
	/** The language in which text in the application is displayed. */
	inline static const Setting<QString>	language								= Setting<QString>	("language",									getDefaultLanguage());
	
	// General/global
	/** Ask user for confirmation before deleting an item. */
	inline static const Setting<bool>	confirmDelete								= Setting<bool>		("confirmDelete",								true);
	/** Ask user for confirmation before discarding changes in an edit dialog. */
	inline static const Setting<bool>	confirmCancel								= Setting<bool>		("confirmCancel",								true);
	/** Allow user to store items with empty names. */
	inline static const Setting<bool>	allowEmptyNames								= Setting<bool>		("allowEmptyNames",								false);
	/** Warn user if an item with the same name already exists. */
	inline static const Setting<bool>	warnAboutDuplicateNames						= Setting<bool>		("warnAboutDuplicateNames",						true);
	
	/** Only prepare the composite table corresponding to the open tab on startup, and defer preparing the other tables until they are opened. */
	inline static const Setting<bool>	onlyPrepareActiveTableOnStartup				= Setting<bool>		("onlyPrepareActiveTableOnStartup",				true);
	
	/** Open the project settings dialog when creating a new database. */
	inline static const Setting<bool>	openProjectSettingsOnNewDatabase			= Setting<bool>		("openProjectSettingsOnNewDatabase",			true);
	
	// Remember UI
	/** Remember the window positions of the main window and all dialogs. */
	inline static const Setting<bool>	rememberWindowPositions						= Setting<bool>		("rememberWindowPositions",						true);
	/** If window positions are remembered, store dialog positions relative to the main window. */
	inline static const Setting<bool>	rememberWindowPositionsRelative				= Setting<bool>		("rememberWindowPositionsRelative",				true);
	/** Remember which tab is open in the main window. */
	inline static const Setting<bool>	rememberTab									= Setting<bool>		("rememberTab",									true);
	/** Remember the column widths of all tables in the main window. */
	inline static const Setting<bool>	rememberColumnWidths						= Setting<bool>		("rememberColumnWidths",						true);
	/** Remember the sorting of all tables in the main window. */
	inline static const Setting<bool>	rememberSorting								= Setting<bool>		("rememberSorting",								true);
	/** Remember the active filters of all tables in the main window. */
	inline static const Setting<bool>	rememberFilters								= Setting<bool>		("rememberFilters",								true);
	
	// Ascent dialog
	/** Enable the date field for new ascents by default. */
	inline static const Setting<bool>	ascentDialog_dateEnabledInitially			= Setting<bool>		("ascentDialog/dateEnabledInitially",			true);
	/** Number of days in the past to set the date for a new ascent to by default. */
	inline static const Setting<int>	ascentDialog_initialDateDaysInPast			= Setting<int>		("ascentDialog/initialDateDaysInPast",			0);
	/** Enable the time field for new ascents by default. */
	inline static const Setting<bool>	ascentDialog_timeEnabledInitially			= Setting<bool>		("ascentDialog/timeEnabledInitially",			false);
	/** Time to set the time for a new ascent to by default. */
	inline static const Setting<QTime>	ascentDialog_initialTime					= Setting<QTime>	("ascentDialog/initialTime",					QTime(12, 00));
	/** Enable the elevation gain field for new ascents by default. */
	inline static const Setting<bool>	ascentDialog_elevationGainEnabledInitially	= Setting<bool>		("ascentDialog/elevationGainEnabledInitially",	true);
	/** Elevation gain to set the elevation gain for a new ascent to by default. */
	inline static const Setting<int>	ascentDialog_initialElevationGain			= Setting<int>		("ascentDialog/initialElevationGain",			500);
	// Peak dialog
	/** Enable the height field for new peaks by default. */
	inline static const Setting<bool>	peakDialog_heightEnabledInitially			= Setting<bool>		("peakDialog/heightEnabledInitially",			true);
	/** Height to set the height for a new peak to by default. */
	inline static const Setting<int>	peakDialog_initialHeight					= Setting<int>		("peakDialog/initialHeight",					2000);
	// Trip dialog
	/** Enable the start and end date fields for new trips by default. */
	inline static const Setting<bool>	tripDialog_datesEnabledInitially			= Setting<bool>		("tripDialog/datesEnabledInitially",			true);
	
	
	// === IMPLICIT ===
	
	// Recently opened databases
	/** The path to the last database that was opened. */
	inline static const Setting<QString>		lastOpenDatabaseFile					= Setting<QString>		("openRecent/lastOpenDatabaseFile");
	/** Paths to the most recently opened databases, in order of most recently opened first. */
	inline static const Setting<QStringList>	recentDatabaseFiles						= Setting<QStringList>	("openRecent/recentDatabaseFiles");
	
	// Window geometry
	/** Remember the main window as maximized. */
	inline static const Setting<bool>			mainWindow_maximized					= Setting<bool>			("implicit/mainWindow/maximized",		false);
	/** Remembered position and size of the main window. */
	inline static const Setting<QRect>			mainWindow_geometry						= Setting<QRect>		("implicit/mainWindow/geometry");
	/** Remembered position and size of the ascent viewer window. */
	inline static const Setting<QRect>			ascentViewer_geometry					= Setting<QRect>		("implicit/ascentViewer/geometry");
	/** Remembered position and size of the settings window. */
	inline static const Setting<QRect>			settingsWindow_geometry					= Setting<QRect>		("implicit/settingsWindow/geometry");
	/** Remembered position and size of the project settings window. */
	inline static const Setting<QRect>			projectSettingsWindow_geometry			= Setting<QRect>		("implicit/projectSettingsWindow/geometry");
	/** Remembered position and size of the ascent dialog. */
	inline static const Setting<QRect>			ascentDialog_geometry					= Setting<QRect>		("implicit/ascentDialog/geometry");
	/** Remembered position and size of the peak dialog. */
	inline static const Setting<QRect>			peakDialog_geometry						= Setting<QRect>		("implicit/peakDialog/geometry");
	/** Remembered position and size of the trip dialog. */
	inline static const Setting<QRect>			tripDialog_geometry						= Setting<QRect>		("implicit/tripDialog/geometry");
	/** Remembered position and size of the hiker dialog. */
	inline static const Setting<QRect>			hikerDialog_geometry					= Setting<QRect>		("implicit/hikerDialog/geometry");
	/** Remembered position and size of the region dialog. */
	inline static const Setting<QRect>			regionDialog_geometry					= Setting<QRect>		("implicit/regionDialog/geometry");
	/** Remembered position and size of the range dialog. */
	inline static const Setting<QRect>			rangeDialog_geometry					= Setting<QRect>		("implicit/rangeDialog/geometry");
	/** Remembered position and size of the country dialog. */
	inline static const Setting<QRect>			countryDialog_geometry					= Setting<QRect>		("implicit/country/geometry");
	
	// View state
	/** Remember filters in the main window as being visible. */
	inline static const Setting<bool>			mainWindow_showFilters					= Setting<bool>			("implicit/mainWindow/showFilters",		true);
	/** Remembered splitter sizes in the ascent viewer. */
	inline static const Setting<QStringList>	ascentViewer_splitterSizes				= Setting<QStringList>	("implicit/ascentViewer/splitterSizes");
	// Open tab
	/** Remembered index of open tab in the main window. */
	inline static const Setting<int>			mainWindow_currentTabIndex				= Setting<int>			("implicit/mainWindow/currentTabIndex",	0);
	
	// Column widths
	/** Remembered column widths of the ascents table in the main window. */
	inline static const Setting<QStringList>	mainWindow_columnWidths_ascentsTable	= Setting<QStringList>	("implicit/mainWindow/columnWidths/ascentsTable");
	/** Remembered column widths of the peaks table in the main window. */
	inline static const Setting<QStringList>	mainWindow_columnWidths_peaksTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/peaksTable");
	/** Remembered column widths of the trips table in the main window. */
	inline static const Setting<QStringList>	mainWindow_columnWidths_tripsTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/tripsTable");
	/** Remembered column widths of the hikers table in the main window. */
	inline static const Setting<QStringList>	mainWindow_columnWidths_hikersTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/hikersTable");
	/** Remembered column widths of the regions table in the main window. */
	inline static const Setting<QStringList>	mainWindow_columnWidths_regionsTable	= Setting<QStringList>	("implicit/mainWindow/columnWidths/regionsTable");
	/** Remembered column widths of the ranges table in the main window. */
	inline static const Setting<QStringList>	mainWindow_columnWidths_rangesTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/rangesTable");
	/** Remembered column widths of the countries table in the main window. */
	inline static const Setting<QStringList>	mainWindow_columnWidths_countriesTable	= Setting<QStringList>	("implicit/mainWindow/columnWidths/countriesTable");
	
	// Sorting
	/** Remembered sorting of the ascents table in the main window. */
	inline static const Setting<QStringList>	mainWindow_sorting_ascentsTable			= Setting<QStringList>	("implicit/mainWindow/sorting/ascentsTable");
	/** Remembered sorting of the peaks table in the main window. */
	inline static const Setting<QStringList>	mainWindow_sorting_peaksTable			= Setting<QStringList>	("implicit/mainWindow/sorting/peaksTable");
	/** Remembered sorting of the trips table in the main window. */
	inline static const Setting<QStringList>	mainWindow_sorting_tripsTable			= Setting<QStringList>	("implicit/mainWindow/sorting/tripsTable");
	/** Remembered sorting of the hikers table in the main window. */
	inline static const Setting<QStringList>	mainWindow_sorting_hikersTable			= Setting<QStringList>	("implicit/mainWindow/sorting/hikersTable");
	/** Remembered sorting of the regions table in the main window. */
	inline static const Setting<QStringList>	mainWindow_sorting_regionsTable			= Setting<QStringList>	("implicit/mainWindow/sorting/regionsTable");
	/** Remembered sorting of the ranges table in the main window. */
	inline static const Setting<QStringList>	mainWindow_sorting_rangesTable			= Setting<QStringList>	("implicit/mainWindow/sorting/rangesTable");
	/** Remembered sorting of the countries table in the main window. */
	inline static const Setting<QStringList>	mainWindow_sorting_countriesTable		= Setting<QStringList>	("implicit/mainWindow/sorting/countriesTable");
	
	// Ascent filters
	/** Remembered active filters of the ascents table in the main window. */
	inline static const Setting<QStringList>	mainWindow_ascentFilters				= Setting<QStringList>	("implicit/mainWindow/filters");
	
	
	
	/**
	 * Clears all settings from the settings file.
	 */
	static inline void resetAll()
	{
		qSettings.clear();
	}
	
	/**
	 * Clears all settings for window positions and sizes from the settings file.
	 */
	static inline void resetGeometrySettings()
	{
		mainWindow_maximized			.clear();
		mainWindow_geometry				.clear();
		ascentViewer_geometry			.clear();
		settingsWindow_geometry			.clear();
		projectSettingsWindow_geometry	.clear();
		ascentDialog_geometry			.clear();
		peakDialog_geometry				.clear();
		tripDialog_geometry				.clear();
		hikerDialog_geometry			.clear();
		regionDialog_geometry			.clear();
		rangeDialog_geometry			.clear();
		countryDialog_geometry			.clear();
	}
	
	/**
	 * Clears all settings for column widths from the settings file.
	 */
	static inline void resetColumnWidths()
	{
		mainWindow_columnWidths_ascentsTable	.clear();
		mainWindow_columnWidths_peaksTable		.clear();
		mainWindow_columnWidths_tripsTable		.clear();
		mainWindow_columnWidths_hikersTable		.clear();
		mainWindow_columnWidths_regionsTable	.clear();
		mainWindow_columnWidths_rangesTable		.clear();
		mainWindow_columnWidths_countriesTable	.clear();
	}
	
private:
	static bool isBelowVersion(QString settingsVersion, QString minimalVersion);
	static bool settingsOlderThan(QString version);
public:
	static QString getAppVersion();
	static void checkForVersionChange();
};



void saveDialogGeometry   (QWidget* dialog, QWidget* parent, const Setting<QRect>* geometrySetting);
void restoreDialogGeometry(QWidget* dialog, QWidget* parent, const Setting<QRect>* geometrySetting);



#endif // SETTINGS_H
