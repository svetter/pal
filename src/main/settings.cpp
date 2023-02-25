#include "settings.h"

#include <QCoreApplication>



Setting::Setting(const QString key, QVariant defaultValue) :
		key(key),
		defaultValue(defaultValue)
{}

QString Setting::lookupAndValidate() const
{
	if (qSettings.contains(key)) {
		QVariant lookupResult = qSettings.value(key);
		bool valueValid = lookupResult.isValid();
		if (valueValid) {
			valueValid &= isValidValue(lookupResult.toString());
		}
		// discard if invalid
		if (!valueValid) qSettings.remove(key);
	}
	// Set default if not set
	if (!qSettings.contains(key)) {
		qSettings.setValue(key, defaultValue);
	}
	return qSettings.value(key).toString();
}



BoolSetting::BoolSetting(const QString key, bool defaultValue) :
		Setting(key, defaultValue)
{}

bool BoolSetting::get() const
{
	return lookupAndValidate().compare("true", Qt::CaseInsensitive) == 0;
}

bool BoolSetting::isValidValue(QString value) const
{
	return !value.isEmpty();
}



IntSetting::IntSetting(const QString key, int defaultValue) :
		Setting(key, defaultValue)
{}

int IntSetting::get() const
{
	bool conversionOk = false;
	int converted = lookupAndValidate().toInt(&conversionOk);
	assert(conversionOk);
	return converted;
}

bool IntSetting::isValidValue(QString value) const
{
	bool conversionOk = false;
	value.toInt(&conversionOk);
	return conversionOk;
}



StringSetting::StringSetting(const QString key, QString defaultValue) :
		Setting(key, defaultValue)
{}

QString StringSetting::get() const
{
	return lookupAndValidate();
}

bool StringSetting::isValidValue(QString value) const
{
	Q_UNUSED(value);
	return true;
}
