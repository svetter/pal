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
 * @file settings.h
 * 
 * This file defines the Setting class, declares the Settings class and defines part of it.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include "src/main/helpers.h"

#include <QDate>
#include <QTime>
#include <QSettings>
#include <QRect>
#include <QWidget>
#include <QDirIterator>
#include <QApplication>
#include <QStyle>
#include <QMainWindow>



/** PAL uses QSettings with INI format to store general (not project-specific) settings. */
inline QSettings qSettings = QSettings(QSettings::IniFormat, QSettings::UserScope, "PeakAscentLogger", "PeakAscentLogger");



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
	inline Setting(const QString& key, QVariant defaultValue = T()) :
		key(key),
		defaultValue(defaultValue)
	{
		assert(defaultValue.canConvert<T>());
	}
	
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
	 * @return	The value of the setting as it is stored in the settings file after validation.
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
	 * @return	The default value of the setting.
	 */
	inline T getDefault() const{
		return defaultValue.value<T>();
	}
	
	/**
	 * Sets the value of the setting.
	 * 
	 * @param value	The new value for the setting.
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



template<typename T>
class MultiSetting
{
	/** Dynamically grown list of all settings in this group, mapped to their sub-keys. */
	QMap<QString, Setting<T>*> settings;
	
	/** The part of the key which all settings share. */
	const QString baseKey;
	/** The default value of all the settings. */
	const QVariant defaultValue;
	
public:
	/**
	 * Creates a new MultiSetting with the given base key and default value.
	 * 
	 * @param baseKey		The common part of the keys under which the settings will be stored.
	 * @param defaultValue	The default value for all the settings.
	 */
	inline MultiSetting(const QString baseKey, QVariant defaultValue = T()) :
		settings(QMap<QString, Setting<T>*>()),
		baseKey(baseKey),
		defaultValue(defaultValue)
	{}

	/**
	 * Destroys the MultiSetting.
	 */
	inline ~MultiSetting()
	{
		qDeleteAll(settings);
	}

	
	
	/**
	 * Checks whether any of the settings are present in the settings file.
	 * 
	 * @return	True if any settings are stored in the settings file under the baseKey, false otherwise.
	 */
	inline bool anyPresent(const QSet<QString>& subKeys)
	{
		for (const QString& subKey : subKeys) {
			createSettingIfMissing(subKey);
			if (settings[subKey].present()) return true;
		}
		return false;
	}
	
	/**
	 * Checks whether none of the settings are present in the settings file.
	 * 
	 * @return	True if no settings are stored in the settings file under the baseKey, false otherwise.
	 */
	inline bool nonePresent(const QSet<QString>& subKeys)
	{
		return !anyPresent(subKeys);
	}
	
	/**
	 * Checks whether al of the the setting are present in the settings file.
	 * 
	 * @param subKeys	The sub-keys of all settings to check.
	 * @return			True if all settings given by their sub-keys are stored in the settings file under the baseKey, false otherwise.
	 */
	inline bool allPresent(const QSet<QString>& subKeys)
	{
		for (const QString& subKey : subKeys) {
			createSettingIfMissing(subKey);
			if (!settings[subKey].present()) return false;
		}
		return true;
	}
	
	/**
	 * Returns the values of the settings after validating them, paired with their sub-keys.
	 * 
	 * If the setting is not present in the settings file or invalid, it is discarded and the
	 * default value is returned and written back to the settings file.
	 * 
	 * @return	The value of the settings as they are stored in the settings file after validation.
	 */
	inline QMap<QString, T> get(const QSet<QString>& subKeys)
	{
		QMap<QString, T> values = QMap<QString, T>();
		for (const QString& subKey : subKeys) {
			createSettingIfMissing(subKey);
			values[subKey] = settings.value(subKey)->get();
		}
		return values;
	}
	
	/**
	 * Returns the default value of the settings.
	 * 
	 * @return	The default value of the settings.
	 */
	inline T getDefault() const
	{
		return defaultValue.value<T>();
	}
	
	
	/**
	 * Sets the value of the setting.
	 * 
	 * @param subKeyValueMap	The new values for the settings as a map of keys and values.
	 */
	inline void set(const QMap<QString, T>& subKeyValueMap)
	{
		for (const auto& [subKey, value] : subKeyValueMap) {
			createSettingIfMissing(subKey);
			settings[subKey]->set(value);
		}
	}
	
	/**
	 * Removes all of the settings from the settings file.
	 */
	inline void clear() const
	{
		qSettings.beginGroup(baseKey);
		for (const QString& key : qSettings.childKeys()) {
			qSettings.remove(key);
		}
		qSettings.endGroup();
	}
	
	
private:
	/**
	 * Creates a new Setting for the given sub-key and adds it to the setttings map.
	 * 
	 * @param subKey	The sub-key for the missing setting
	 */
	inline void createSettingIfMissing(const QString& subKey)
	{
		if (!settings.contains(subKey)) {
			settings.insert(subKey, new Setting<T>(baseKey + "/" + subKey, defaultValue));
		}
	}
};



/**
 * A class to globally store all settings.
 * 
 * This class contains all settings that are not project-specific.
 */
class Settings {
public:
	/** The style which was applied to the application at launch. */
	inline static QString systemDefaultStyle;
	
	
	// === APP VERSION ===
	
	/** The version of the application with which the settings file was last written. */
	inline static const Setting<QString>		appVersion									= Setting<QString>		("appVersion",									getAppVersion());
	
	
	// === EXPLICIT ===
	
	// Language
	/** The language in which text in the application is displayed. */
	inline static const Setting<QString>		language									= Setting<QString>		("language",									getDefaultLanguageCode());
	
	// Application UI style
	/** The style in which the application is displayed. */
	inline static const Setting<QString>		uiStyle										= Setting<QString>		("uiStyle",										"");
	/** The color scheme in which the application is displayed: bright, dark, or use system default. */
	inline static const Setting<QString>		uiColorScheme								= Setting<QString>		("uiColorScheme",								"system");
	
	// General/global
	/** Ask user for confirmation before deleting an item. */
	inline static const Setting<bool>			confirmDelete								= Setting<bool>			("confirmDelete",								true);
	/** Ask user for confirmation before discarding changes in an edit dialog. */
	inline static const Setting<bool>			confirmCancel								= Setting<bool>			("confirmCancel",								true);
	/** Warn user if an item with the same name already exists. */
	inline static const Setting<bool>			warnAboutDuplicateNames						= Setting<bool>			("warnAboutDuplicateNames",						true);
	
	/** For any integer, date or time column in any table, default to sorting them in descending order first. */
	inline static const Setting<bool>			sortNumericColumnsDescendingByDefault		= Setting<bool>			("sortNumericColumnsDescendingByDefault",		false);
	
	/** Only prepare the composite table corresponding to the open tab on startup, and defer preparing the other tables until they are opened. */
	inline static const Setting<bool>			onlyPrepareActiveTableOnStartup				= Setting<bool>			("onlyPrepareActiveTableOnStartup",				true);
	
	// Remember UI
	/** Remember the window positions of the main window and all dialogs. */
	inline static const Setting<bool>			rememberWindowPositions						= Setting<bool>			("rememberWindowPositions",						true);
	/** If window positions are remembered, store dialog positions relative to the main window. */
	inline static const Setting<bool>			rememberWindowPositionsRelative				= Setting<bool>			("rememberWindowPositionsRelative",				true);
	/** Remember which tab is open in the main window. */
	inline static const Setting<bool>			rememberTab									= Setting<bool>			("rememberTab",									true);
	/** Remember the column widths of all tables in the main window. */
	inline static const Setting<bool>			rememberColumnWidths						= Setting<bool>			("rememberColumnWidths",						true);
	/** Remember the column order of all tables in the main window. */
	inline static const Setting<bool>			rememberColumnOrder							= Setting<bool>			("rememberColumnOrder",							true);
	/** Remember which columns are hidden in all tables in the main window. */
	inline static const Setting<bool>			rememberHiddenColumns						= Setting<bool>			("rememberHiddenColumns",						true);
	/** Remember the sorting of all tables in the main window. */
	inline static const Setting<bool>			rememberSorting								= Setting<bool>			("rememberSorting",								true);
	/** Remember the active filters of all tables in the main window. */
	inline static const Setting<bool>			rememberFilters								= Setting<bool>			("rememberFilters",								true);
	
	// Ascent dialog
	/** Enable the date field for new ascents by default. */
	inline static const Setting<bool>			ascentDialog_dateEnabledInitially			= Setting<bool>			("ascentDialog/dateEnabledInitially",			true);
	/** Number of days in the past to set the date for a new ascent to by default. */
	inline static const Setting<int>			ascentDialog_initialDateDaysInPast			= Setting<int>			("ascentDialog/initialDateDaysInPast",			0);
	/** Enable the time field for new ascents by default. */
	inline static const Setting<bool>			ascentDialog_timeEnabledInitially			= Setting<bool>			("ascentDialog/timeEnabledInitially",			false);
	/** Time to set the time for a new ascent to by default. */
	inline static const Setting<QTime>			ascentDialog_initialTime					= Setting<QTime>		("ascentDialog/initialTime",					QTime(12, 00));
	/** Enable the elevation gain field for new ascents by default. */
	inline static const Setting<bool>			ascentDialog_elevationGainEnabledInitially	= Setting<bool>			("ascentDialog/elevationGainEnabledInitially",	true);
	/** Elevation gain to set the elevation gain for a new ascent to by default. */
	inline static const Setting<int>			ascentDialog_initialElevationGain			= Setting<int>			("ascentDialog/initialElevationGain",			500);
	// Peak dialog
	/** Enable the height field for new peaks by default. */
	inline static const Setting<bool>			peakDialog_heightEnabledInitially			= Setting<bool>			("peakDialog/heightEnabledInitially",			true);
	/** Height to set the height for a new peak to by default. */
	inline static const Setting<int>			peakDialog_initialHeight					= Setting<int>			("peakDialog/initialHeight",					2000);
	// Trip dialog
	/** Enable the start and end date fields for new trips by default. */
	inline static const Setting<bool>			tripDialog_datesEnabledInitially			= Setting<bool>			("tripDialog/datesEnabledInitially",			true);
	
	// Google API key
	/** The Google API key to use for finding links for peaks. */
	inline static const Setting<QString>		googleApiKey								= Setting<QString>		("googleApiKey",								"");
	
	// Ascent viewer
	/** Show the ascent title below the peak name in the peak info box instead of the ascent info box in the ascent viewer. */
	inline static const Setting<bool>			ascentViewer_ascentTitleUnderPeakName		= Setting<bool>			("ascentViewer/ascentTitleUnderPeakName",		false);
	
	
	// === IMPLICIT ===
	
	// Recently opened databases
	/** The path to the last database that was opened. */
	inline static const	Setting<QString>		lastOpenDatabaseFile						= Setting<QString>		("openRecent/lastOpenDatabaseFile");
	/** Paths to the most recently opened databases, in order of most recently opened first. */
	inline static const	Setting<QStringList>	recentDatabaseFiles							= Setting<QStringList>	("openRecent/recentDatabaseFiles");
	
	// Window geometry
	/** Remember the main window as maximized. */
	inline static const	Setting<bool>			mainWindow_maximized						= Setting<bool>			("implicit/mainWindow/maximized",				false);
	/** Remembered position and size of the main window. */
	inline static const	Setting<QRect>			mainWindow_geometry							= Setting<QRect>		("implicit/mainWindow/geometry");
	/** Remembered position and size of the ascent viewer window. */
	inline static const	Setting<QRect>			ascentViewer_geometry						= Setting<QRect>		("implicit/ascentViewer/geometry");
	/** Remembered position and size of the settings window. */
	inline static const	Setting<QRect>			settingsWindow_geometry						= Setting<QRect>		("implicit/settingsWindow/geometry");
	/** Remembered position and size of the project settings window. */
	inline static const	Setting<QRect>			projectSettingsWindow_geometry				= Setting<QRect>		("implicit/projectSettingsWindow/geometry");
	/** Remembered position and size of the ascent dialog. */
	inline static const	Setting<QRect>			ascentDialog_geometry						= Setting<QRect>		("implicit/ascentDialog/geometry");
	/** Remembered position and size of the peak dialog. */
	inline static const	Setting<QRect>			peakDialog_geometry							= Setting<QRect>		("implicit/peakDialog/geometry");
	/** Remembered position and size of the trip dialog. */
	inline static const	Setting<QRect>			tripDialog_geometry							= Setting<QRect>		("implicit/tripDialog/geometry");
	/** Remembered position and size of the hiker dialog. */
	inline static const	Setting<QRect>			hikerDialog_geometry						= Setting<QRect>		("implicit/hikerDialog/geometry");
	/** Remembered position and size of the region dialog. */
	inline static const	Setting<QRect>			regionDialog_geometry						= Setting<QRect>		("implicit/regionDialog/geometry");
	/** Remembered position and size of the range dialog. */
	inline static const	Setting<QRect>			rangeDialog_geometry						= Setting<QRect>		("implicit/rangeDialog/geometry");
	/** Remembered position and size of the country dialog. */
	inline static const	Setting<QRect>			countryDialog_geometry						= Setting<QRect>		("implicit/countryDialog/geometry");
	
	// Item stats panel
	/** Remembered state of the view setting to pin the axis ranges in the item-related statistics panels. */
	inline static const Setting<bool>			itemStats_pinRanges							= Setting<bool>			("implicit/mainWindow/itemStats/pinRanges",		false);
	/** Remembered visibility state for the statistics panel in the ascents tab of the main window. */
	inline static const	Setting<bool>			ascentsStats_show							= Setting<bool>			("implicit/mainWindow/ascents/showStats",		true);
	/** Remembered visibility state for the statistics panel in the peaks tab of the main window. */
	inline static const	Setting<bool>			peaksStats_show								= Setting<bool>			("implicit/mainWindow/peaks/showStats",			true);
	/** Remembered visibility state for the statistics panel in the trips tab of the main window. */
	inline static const	Setting<bool>			tripsStats_show								= Setting<bool>			("implicit/mainWindow/trips/showStats",			true);
	/** Remembered visibility state for the statistics panel in the hikers tab of the main window. */
	inline static const	Setting<bool>			hikersStats_show							= Setting<bool>			("implicit/mainWindow/hikers/showStats",		true);
	/** Remembered visibility state for the statistics panel in the regions tab of the main window. */
	inline static const	Setting<bool>			regionsStats_show							= Setting<bool>			("implicit/mainWindow/regions/showStats",		true);
	/** Remembered visibility state for the statistics panel in the ranges tab of the main window. */
	inline static const	Setting<bool>			rangesStats_show							= Setting<bool>			("implicit/mainWindow/ranges/showStats",		true);
	/** Remembered visibility state for the statistics panel in the countries tab of the main window. */
	inline static const	Setting<bool>			countriesStats_show							= Setting<bool>			("implicit/mainWindow/countries/showStats",		true);
	
	// Stats panel splitter sizes
	/** Remembered sizes for the splitter between table and statistics frame in the ascents tab of the main window. */
	inline static const	Setting<QStringList>	ascentsStats_splitterSizes					= Setting<QStringList>	("implicit/mainWindow/ascents/splitterSizes");
	/** Remembered sizes for the splitter between table and statistics frame in the peaks tab of the main window. */
	inline static const	Setting<QStringList>	peaksStats_splitterSizes					= Setting<QStringList>	("implicit/mainWindow/peaks/splitterSizes");
	/** Remembered sizes for the splitter between table and statistics frame in the trips tab of the main window. */
	inline static const	Setting<QStringList>	tripsStats_splitterSizes					= Setting<QStringList>	("implicit/mainWindow/trips/splitterSizes");
	/** Remembered sizes for the splitter between table and statistics frame in the hikers tab of the main window. */
	inline static const	Setting<QStringList>	hikersStats_splitterSizes					= Setting<QStringList>	("implicit/mainWindow/hikers/splitterSizes");
	/** Remembered sizes for the splitter between table and statistics frame in the regions tab of the main window. */
	inline static const	Setting<QStringList>	regionsStats_splitterSizes					= Setting<QStringList>	("implicit/mainWindow/regions/splitterSizes");
	/** Remembered sizes for the splitter between table and statistics frame in the ranges tab of the main window. */
	inline static const	Setting<QStringList>	rangesStats_splitterSizes					= Setting<QStringList>	("implicit/mainWindow/ranges/splitterSizes");
	/** Remembered sizes for the splitter between table and statistics frame in the countries tab of the main window. */
	inline static const	Setting<QStringList>	countriesStats_splitterSizes				= Setting<QStringList>	("implicit/mainWindow/countries/splitterSizes");
	
	// Ascent viewer
	/** Remembered value of the spin box determining the interval (in seconds) for a slideshow in the ascent viewer. */
	inline static const Setting<int>			ascentViewer_slideshowInterval				= Setting<int>			("implicit/ascentViewer/slideshowInterval",		6);
	/** Remembered value of the checkbox determining whether the slideshow should be started automatically in the ascent viewer. */
	inline static const Setting<bool>			ascentViewer_slideshowAutostart				= Setting<bool>			("implicit/ascentViewer/slideshowAutostart",	false);
	
	// Ascent viewer splitter sizes
	/** Remembered sizes for the left splitter in the ascent viewer. */
	inline static const	Setting<QStringList>	ascentViewer_leftSplitterSizes				= Setting<QStringList>	("implicit/ascentViewer/leftSplitterSizes");
	/** Remembered sizes for the splitter dividing the two description boxes in the ascent viewer. */
	inline static const	Setting<QStringList>	ascentViewer_descriptionSplitterSizes		= Setting<QStringList>	("implicit/ascentViewer/descriptionSplitterSizes");
	/** Remembered sizes for the right splitter in the ascent viewer. */
	inline static const	Setting<QStringList>	ascentViewer_rightSplitterSizes				= Setting<QStringList>	("implicit/ascentViewer/rightSplitterSizes");
	
	
	
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
	
private:
	static bool settingsVersionUpTo(const QString& version);
public:
	static void checkForVersionChange();
};



// Saving/restoring implicit settings

void saveDialogGeometry(QWidget& dialog, QMainWindow& mainWindow, const Setting<QRect>& geometrySetting);
void restoreDialogGeometry(QWidget& dialog, QMainWindow& mainWindow, const Setting<QRect>& geometrySetting);

void saveSplitterSizes(QSplitter& splitter, const Setting<QStringList>& splitterSizesSetting);
void restoreSplitterSizes(QSplitter& splitter, const Setting<QStringList>& splitterSizesSetting);



#endif // SETTINGS_H
