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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDate>
#include <QTime>
#include <QSettings>
#include <QRect>
#include <QWidget>
#include <QDirIterator>



inline QSettings qSettings = QSettings(QSettings::IniFormat, QSettings::UserScope, "PeakAscentLogger", "PeakAscentLogger");



QPair<QStringList, QStringList> getSupportedLanguages();
QString getDefaultLanguage();



template<typename T>
class Setting {
	const QString key;
	const QVariant defaultValue;
	
public:
	inline Setting(const QString key, QVariant defaultValue = QVariant()) :
			key(key),
			defaultValue(defaultValue)
	{}
	
	inline bool present() const
	{
		return qSettings.contains(key);
	}
	
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
	
	inline T getDefault() const{
		return defaultValue.value<T>();
	}
	
	inline void set(T value) const
	{
		qSettings.setValue(key, QVariant::fromValue(value));
	}
	
	inline void clear() const
	{
		qSettings.remove(key);
	}
};



class Settings {
public:
	// === APP VERSION ===
	inline static const Setting<QString>	appVersion								= Setting<QString>	("appVersion",									"0.0.0");
	
	
	// === EXPLICIT ===
	
	// Language
	inline static const Setting<QString>	language								= Setting<QString>	("language",									getDefaultLanguage());
	
	// General/global
	inline static const Setting<bool>	confirmDelete								= Setting<bool>		("confirmDelete",								true);
	inline static const Setting<bool>	confirmCancel								= Setting<bool>		("confirmCancel",								true);
	inline static const Setting<bool>	allowEmptyNames								= Setting<bool>		("allowEmptyNames",								false);
	inline static const Setting<bool>	warnAboutDuplicateNames						= Setting<bool>		("warnAboutDuplicateNames",						true);
	
	inline static const Setting<bool>	onlyPrepareActiveTableOnStartup				= Setting<bool>		("onlyPrepareActiveTableOnStartup",				true);
	
	inline static const Setting<bool>	openProjectSettingsOnNewDatabase			= Setting<bool>		("openProjectSettingsOnNewDatabase",			true);
	
	// Remember UI
	inline static const Setting<bool>	rememberWindowPositions						= Setting<bool>		("rememberWindowPositions",						true);
	inline static const Setting<bool>	rememberWindowPositionsRelative				= Setting<bool>		("rememberWindowPositionsRelative",				true);
	inline static const Setting<bool>	rememberTab									= Setting<bool>		("rememberTab",									true);
	inline static const Setting<bool>	rememberColumnWidths						= Setting<bool>		("rememberColumnWidths",						true);
	inline static const Setting<bool>	rememberSorting								= Setting<bool>		("rememberSorting",								true);
	inline static const Setting<bool>	rememberFilters								= Setting<bool>		("rememberFilters",								true);
	
	// Ascent dialog
	inline static const Setting<bool>	ascentDialog_dateEnabledInitially			= Setting<bool>		("ascentDialog/dateEnabledInitially",			true);
	inline static const Setting<int>	ascentDialog_initialDateDaysInPast			= Setting<int>		("ascentDialog/initialDateDaysInPast",			0);
	inline static const Setting<bool>	ascentDialog_timeEnabledInitially			= Setting<bool>		("ascentDialog/timeEnabledInitially",			false);
	inline static const Setting<QTime>	ascentDialog_initialTime					= Setting<QTime>	("ascentDialog/initialTime",					QTime(12, 00));
	inline static const Setting<bool>	ascentDialog_elevationGainEnabledInitially	= Setting<bool>		("ascentDialog/elevationGainEnabledInitially",	true);
	inline static const Setting<int>	ascentDialog_initialElevationGain			= Setting<int>		("ascentDialog/initialElevationGain",			500);
	// Peak dialog
	inline static const Setting<bool>	peakDialog_heightEnabledInitially			= Setting<bool>		("peakDialog/heightEnabledInitially",			true);
	inline static const Setting<int>	peakDialog_initialHeight					= Setting<int>		("peakDialog/initialHeight",					2000);
	// Trip dialog
	inline static const Setting<bool>	tripDialog_datesEnabledInitially			= Setting<bool>		("tripDialog/datesEnabledInitially",			true);
	
	
	// === IMPLICIT ===
	
	// Recently opened databases
	inline static const Setting<QString>		lastOpenDatabaseFile					= Setting<QString>		("openRecent/lastOpenDatabaseFile");
	inline static const Setting<QStringList>	recentDatabaseFiles						= Setting<QStringList>	("openRecent/recentDatabaseFiles");
	
	// Window geometry
	inline static const Setting<bool>			mainWindow_maximized					= Setting<bool>			("implicit/mainWindow/maximized",		false);
	inline static const Setting<QRect>			mainWindow_geometry						= Setting<QRect>		("implicit/mainWindow/geometry");
	inline static const Setting<QRect>			ascentViewer_geometry					= Setting<QRect>		("implicit/ascentViewer/geometry");
	inline static const Setting<QRect>			settingsWindow_geometry					= Setting<QRect>		("implicit/settingsWindow/geometry");
	inline static const Setting<QRect>			projectSettingsWindow_geometry			= Setting<QRect>		("implicit/projectSettingsWindow/geometry");
	inline static const Setting<QRect>			ascentDialog_geometry					= Setting<QRect>		("implicit/ascentDialog/geometry");
	inline static const Setting<QRect>			peakDialog_geometry						= Setting<QRect>		("implicit/peakDialog/geometry");
	inline static const Setting<QRect>			tripDialog_geometry						= Setting<QRect>		("implicit/tripDialog/geometry");
	inline static const Setting<QRect>			hikerDialog_geometry					= Setting<QRect>		("implicit/hikerDialog/geometry");
	inline static const Setting<QRect>			regionDialog_geometry					= Setting<QRect>		("implicit/regionDialog/geometry");
	inline static const Setting<QRect>			rangeDialog_geometry					= Setting<QRect>		("implicit/rangeDialog/geometry");
	inline static const Setting<QRect>			countryDialog_geometry					= Setting<QRect>		("implicit/country/geometry");
	
	// View state
	inline static const Setting<bool>			mainWindow_showFilters					= Setting<bool>			("implicit/mainWindow/showFilters",		true);
	inline static const Setting<QStringList>	ascentViewer_splitterSizes				= Setting<QStringList>	("implicit/ascentViewer/splitterSizes");
	// Open tab
	inline static const Setting<int>			mainWindow_currentTabIndex				= Setting<int>			("implicit/mainWindow/currentTabIndex",	0);
	
	// Column widths
	inline static const Setting<QStringList>	mainWindow_columnWidths_ascentsTable	= Setting<QStringList>	("implicit/mainWindow/columnWidths/ascentsTable");
	inline static const Setting<QStringList>	mainWindow_columnWidths_peaksTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/peaksTable");
	inline static const Setting<QStringList>	mainWindow_columnWidths_tripsTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/tripsTable");
	inline static const Setting<QStringList>	mainWindow_columnWidths_hikersTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/hikersTable");
	inline static const Setting<QStringList>	mainWindow_columnWidths_regionsTable	= Setting<QStringList>	("implicit/mainWindow/columnWidths/regionsTable");
	inline static const Setting<QStringList>	mainWindow_columnWidths_rangesTable		= Setting<QStringList>	("implicit/mainWindow/columnWidths/rangesTable");
	inline static const Setting<QStringList>	mainWindow_columnWidths_countriesTable	= Setting<QStringList>	("implicit/mainWindow/columnWidths/countriesTable");
	
	// Sorting
	inline static const Setting<QStringList>	mainWindow_sorting_ascentsTable			= Setting<QStringList>	("implicit/mainWindow/sorting/ascentsTable");
	inline static const Setting<QStringList>	mainWindow_sorting_peaksTable			= Setting<QStringList>	("implicit/mainWindow/sorting/peaksTable");
	inline static const Setting<QStringList>	mainWindow_sorting_tripsTable			= Setting<QStringList>	("implicit/mainWindow/sorting/tripsTable");
	inline static const Setting<QStringList>	mainWindow_sorting_hikersTable			= Setting<QStringList>	("implicit/mainWindow/sorting/hikersTable");
	inline static const Setting<QStringList>	mainWindow_sorting_regionsTable			= Setting<QStringList>	("implicit/mainWindow/sorting/regionsTable");
	inline static const Setting<QStringList>	mainWindow_sorting_rangesTable			= Setting<QStringList>	("implicit/mainWindow/sorting/rangesTable");
	inline static const Setting<QStringList>	mainWindow_sorting_countriesTable		= Setting<QStringList>	("implicit/mainWindow/sorting/countriesTable");
	
	// Ascent filters
	inline static const Setting<QStringList>	mainWindow_ascentFilters				= Setting<QStringList>	("implicit/mainWindow/filters");
	
	
	
	static inline void resetAll()
	{
		qSettings.clear();
	}
	
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
