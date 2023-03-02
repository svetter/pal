#ifndef SETTINGS_H
#define SETTINGS_H

#include "qdatetime.h"
#include <QSettings>
#include <QRect>



inline static QSettings qSettings = QSettings(QSettings::IniFormat, QSettings::UserScope, "PeakAscentLogger", "PeakAscentLogger");



template<typename T>
class Setting {
	const QString key;
	const QVariant defaultValue;
	
public:
	inline Setting(const QString key, QVariant defaultValue = QVariant()) :
			key(key),
			defaultValue(defaultValue)
	{}
	
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
	// === EXPLICIT ===
	
	// General/global
	inline static const Setting<bool>	confirmDelete								= Setting<bool>		("confirmDelete",								true);
	inline static const Setting<bool>	confirmCancel								= Setting<bool>		("confirmCancel",								true);
	inline static const Setting<bool>	allowEmptyNames								= Setting<bool>		("allowEmptyNames",								false);
	
	inline static const Setting<bool>	openProjectSettingsOnNewDatabase			= Setting<bool>		("openProjectSettingsOnNewDatabase",			true);
	
	inline static const Setting<bool>	rememberWindowPositions						= Setting<bool>		("rememberWindowPositions",						true);
	// Main window
	inline static const Setting<bool>	mainWindow_rememberTab						= Setting<bool>		("mainWindow/rememberTab",						true);
	inline static const Setting<bool>	mainWindow_rememberColumnWidths				= Setting<bool>		("mainWindow/rememberColumnWidths",				true);
	
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
	inline static const Setting<QRect>			settingsWindow_geometry					= Setting<QRect>		("implicit/settingsWindow/geometry");
	inline static const Setting<QRect>			projectSettingsWindow_geometry			= Setting<QRect>		("implicit/projectSettingsWindow/geometry");
	inline static const Setting<QRect>			ascentDialog_geometry					= Setting<QRect>		("implicit/ascentDialog/geometry");
	inline static const Setting<QRect>			peakDialog_geometry						= Setting<QRect>		("implicit/peakDialog/geometry");
	inline static const Setting<QRect>			tripDialog_geometry						= Setting<QRect>		("implicit/tripDialog/geometry");
	inline static const Setting<QRect>			hikerDialog_geometry					= Setting<QRect>		("implicit/hikerDialog/geometry");
	inline static const Setting<QRect>			regionDialog_geometry					= Setting<QRect>		("implicit/regionDialog/geometry");
	inline static const Setting<QRect>			rangeDialog_geometry					= Setting<QRect>		("implicit/rangeDialog/geometry");
	inline static const Setting<QRect>			countryDialog_geometry					= Setting<QRect>		("implicit/country/geometry");
	
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
	
	
	
	inline void resetAll()
	{
		qSettings.clear();
	}
};



#endif // SETTINGS_H