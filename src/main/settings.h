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
};



#endif // SETTINGS_H
