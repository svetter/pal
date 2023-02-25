#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>



class Setting {	
	inline static QSettings qSettings = QSettings(QSettings::IniFormat, QSettings::UserScope, "PeakAscentLogger", "PeakAscentLogger");
	
	const QString key;
	const QVariant defaultValue;
	
protected:
	Setting(const QString key, QVariant defaultValue);
	
	QString lookupAndValidate() const;
	
private:
	virtual bool isValidValue(QString value) const = 0;
};



class BoolSetting : protected Setting {
public:
	BoolSetting(const QString key, bool defaultValue);
	
	bool get() const;
	
private:
	virtual bool isValidValue(QString value) const override;
};


class IntSetting : protected Setting {
public:
	IntSetting(const QString key, int defaultValue);
	
	int get() const;
	
private:
	virtual bool isValidValue(QString value) const override;
};


class StringSetting : protected Setting {
public:
	StringSetting(const QString key, QString defaultValue);
	
	QString get() const;
	
private:
	virtual bool isValidValue(QString value) const override;
};



class Settings {
public:
	inline static const BoolSetting	showCancelWarnings							= BoolSetting	("showCancelWarnings",							true);
	inline static const BoolSetting	showDeleteWarnings							= BoolSetting	("showDeleteWarnings",							true);
	inline static const BoolSetting	allowEmptyNames								= BoolSetting	("allowEmptyNames",								false);
	
	inline static const BoolSetting	ascentDialog_dateEnabledByDefault			= BoolSetting	("ascentDialog/dateEnabledByDefault",			true);
	inline static const IntSetting	ascentDialog_initialDateDaysInPast			= IntSetting	("ascentDialog/initialDateDaysInPast",			0);
	inline static const BoolSetting	ascentDialog_timeEnabledByDefault			= BoolSetting	("ascentDialog/timeEnabledByDefault",			false);
	inline static const BoolSetting	ascentDialog_elevationGainEnabledByDefault	= BoolSetting	("ascentDialog/elevationGainEnabledByDefault",	true);
	
	inline static const BoolSetting	peakDialog_heightEnabledByDefault			= BoolSetting	("peakDialog/heightEnabledByDefault",			true);
};



#endif // SETTINGS_H
