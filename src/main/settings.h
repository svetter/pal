#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QRect>



template<typename T>
class Setting {
	inline static QSettings qSettings = QSettings(QSettings::IniFormat, QSettings::UserScope, "PeakAscentLogger", "PeakAscentLogger");
	
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
				qSettings.remove(key);
				qDebug() << "Discarded invalid setting" << key << lookupResult;
			}
		}
		
		// Set default if not set
		if (!qSettings.contains(key) && defaultValue.isValid()) {
			qSettings.setValue(key, defaultValue);
		}
		
		return qSettings.value(key).value<T>();
	}
	
	inline void set(T value) const
	{
		qSettings.setValue(key, QVariant::fromValue(value));
	}
};



class Settings {
public:
	// General/global
	inline static const Setting<bool>	showCancelWarnings							= Setting<bool>		("showCancelWarnings",							true);
	inline static const Setting<bool>	showDeleteWarnings							= Setting<bool>		("showDeleteWarnings",							true);
	inline static const Setting<bool>	allowEmptyNames								= Setting<bool>		("allowEmptyNames",								false);
	
	// Ascent dialog
	inline static const Setting<bool>	ascentDialog_dateEnabledByDefault			= Setting<bool>		("ascentDialog/dateEnabledByDefault",			true);
	inline static const Setting<int>	ascentDialog_initialDateDaysInPast			= Setting<int>		("ascentDialog/initialDateDaysInPast",			0);
	inline static const Setting<bool>	ascentDialog_timeEnabledByDefault			= Setting<bool>		("ascentDialog/timeEnabledByDefault",			false);
	inline static const Setting<bool>	ascentDialog_elevationGainEnabledByDefault	= Setting<bool>		("ascentDialog/elevationGainEnabledByDefault",	true);
	// Peak dialog
	inline static const Setting<bool>	peakDialog_heightEnabledByDefault			= Setting<bool>		("peakDialog/heightEnabledByDefault",			true);
	
	// Implicit: Window geometry
	inline static const Setting<bool>	mainWindow_maximized						= Setting<bool>		("implicit/mainWindow/maximized",				false);
	inline static const Setting<QRect>	mainWindow_geometry							= Setting<QRect>	("implicit/mainWindow/geometry");
	inline static const Setting<QRect>	ascentDialog_geometry						= Setting<QRect>	("implicit/ascentDialog/geometry");
	inline static const Setting<QRect>	peakDialog_geometry							= Setting<QRect>	("implicit/peakDialog/geometry");
	inline static const Setting<QRect>	tripDialog_geometry							= Setting<QRect>	("implicit/tripDialog/geometry");
	inline static const Setting<QRect>	hikerDialog_geometry						= Setting<QRect>	("implicit/hikerDialog/geometry");
	inline static const Setting<QRect>	regionDialog_geometry						= Setting<QRect>	("implicit/regionDialog/geometry");
	inline static const Setting<QRect>	rangeDialog_geometry						= Setting<QRect>	("implicit/rangeDialog/geometry");
	inline static const Setting<QRect>	countryDialog_geometry						= Setting<QRect>	("implicit/country/geometry");
};



#endif // SETTINGS_H
