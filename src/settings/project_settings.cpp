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
 * @file project_settings.cpp
 * 
 * This file defines the GenericProjectSetting, ProjectSetting and ProjectMultiSetting classes.
 */

#include "project_settings.h"



/**
 * Creates a new GenericProjectSetting.
 * 
 * @param table			The project settings table.
 * @param key			The key of the setting.
 * @param defaultValue	The default value for the setting.
 */
GenericProjectSetting::GenericProjectSetting(SettingsTable* table, const QString& key, QVariant defaultValue) :
	table(table),
	key(key),
	defaultValue(defaultValue)
{}



/**
 * Indicates whether the setting is present.
 * 
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			True if the setting is present in the project settings storage, false otherwise.
 */
bool GenericProjectSetting::present(QWidget* parent) const
{
	return table->settingIsPresent(this, parent);
}

/**
 * Returns the current value of the setting in the project settings storage as a QVariant.
 * 
 * If the parent window is given, the setting is added to the project settings storage using the
 * default value if it is not present.
 * 
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			The current value of the setting as a QVariant.
 */
QVariant GenericProjectSetting::getAsQVariant(QWidget* parent) const
{
	return table->getSetting(this, parent);
}

/**
 * Returns the default value for the setting as a QVariant.
 * 
 * @return	The default value for the setting as a QVariant.
 */
QVariant GenericProjectSetting::getDefaultAsQVariant() const
{
	return defaultValue;
}


/**
 * Sets the given value for the setting in the project settings storage.
 * 
 * @param parent	The parent window. Cannot be nullptr.
 * @param value		The new value for the setting.
 */
void GenericProjectSetting::set(QWidget* parent, QVariant value) const
{
	table->setSetting(parent, this, value);
}

/**
 * Clears the setting in the project settings storage.
 *
 * @param parent	The parent window. Cannot be nullptr.
 */
void GenericProjectSetting::clear(QWidget* parent) const
{
	table->clearSetting(parent, this);
}

/**
 * Removes the setting from the project settings storage completely.
 *
 * @param parent	The parent window. Cannot be nullptr.
 */
void GenericProjectSetting::remove(QWidget* parent) const
{
	table->removeSetting(parent, this);
}





/**
 * Creates a new GenericProjectSetting.
 * 
 * @param settingsTable	The project settings table.
 * @param key			The key of the setting.
 * @param defaultValue	The default value for the setting.
 */
template<typename T>
ProjectSetting<T>::ProjectSetting(SettingsTable* table, const QString& key, QVariant defaultValue) :
	GenericProjectSetting(table, key, defaultValue)
{
	assert(defaultValue.canConvert<T>());
}



/**
 * Returns the current value of the setting in the project settings storage.
 * 
 * If the parent window is given, the setting is added to the project settings storage using the
 * default value if it is not present.
 * 
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			The current value of the setting, or the default value if the setting is not present.
 */
template<typename T>
T ProjectSetting<T>::get(QWidget* parent) const
{
	QVariant value = table->getSetting(this, parent);
	assert(value.canConvert<T>());
	return value.value<T>();
}

/**
 * Returns the default value for the setting.
 * 
 * @return	The default value for the setting.
 */
template<typename T>
T ProjectSetting<T>::getDefault() const
{
	return defaultValue.value<T>();
}



// List used types as compiler hints
template class ProjectSetting<bool>;
template class ProjectSetting<int>;
template class ProjectSetting<QString>;
template class ProjectSetting<QDate>;





/**
 * Creates a new ProjectMultiSetting with the given base key and default value.
 * 
 * @param baseKey		The common part of the keys under which the settings will be stored.
 * @param defaultValue	The default value for all the settings.
 */
template<typename T>
ProjectMultiSetting<T>::ProjectMultiSetting(SettingsTable* table, const QString baseKey, QVariant defaultValue) :
	settings(QMap<QString, ProjectSetting<T>*>()),
	table(table),
	baseKey(baseKey),
	defaultValue(defaultValue)
{}

/**
 * Destroys the ProjectMultiSetting.
 */
 template<typename T>
 ProjectMultiSetting<T>::~ProjectMultiSetting()
{
	qDeleteAll(settings);
}


/**
 * Checks whether any of the settings are present in the project settings storage.
 * 
 * @return	True if any settings are stored in the settings file under the baseKey, false otherwise.
 */
template<typename T>
bool ProjectMultiSetting<T>::anyPresent(const QSet<QString>& subKeys)
{
	for (const QString& subKey : subKeys) {
		createSettingIfMissing(subKey);
		if (settings[subKey]->present()) return true;
	}
	return false;
}

/**
 * Checks whether none of the settings are present in the project settings storage.
 * 
 * @return	True if no settings are stored in the settings file under the baseKey, false otherwise.
 */
template<typename T>
bool ProjectMultiSetting<T>::nonePresent(const QSet<QString>& subKeys)
{
	return !anyPresent(subKeys);
}

/**
 * Checks whether al of the the setting are present in the project settings storage.
 * 
 * @param subKeys	The sub-keys of all settings to check.
 * @return			True if all settings given by their sub-keys are stored in the settings file under the baseKey, false otherwise.
 */
template<typename T>
bool ProjectMultiSetting<T>::allPresent(const QSet<QString>& subKeys)
{
	for (const QString& subKey : subKeys) {
		createSettingIfMissing(subKey);
		if (!settings[subKey]->present()) return false;
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
template<typename T>
QMap<QString, T> ProjectMultiSetting<T>::get(const QSet<QString>& subKeys)
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
template<typename T>
T ProjectMultiSetting<T>::getDefault() const
{
	return defaultValue.value<T>();
}


/**
 * Sets the value of the setting.
 * 
 * @param values	The new values for the settings.
 */
template<typename T>
void ProjectMultiSetting<T>::set(QWidget* parent, const QMap<QString, T>& subKeyValueMap)
{
	for (const auto& [subKey, value] : subKeyValueMap.asKeyValueRange()) {
		createSettingIfMissing(subKey);
		settings[subKey]->set(parent, value);
	}
}

/**
 * Removes all of the settings from the project settings storage.
 */
template<typename T>
void ProjectMultiSetting<T>::clear(QWidget* parent, SettingsTable* settingsTable) const
{
	settingsTable->clearAllSettings(parent, baseKey);
}


/**
 * Creates a new Setting for the given sub-key and adds it to the setttings map.
 * 
 * @param subKey	The sub-key for the missing setting
 */
template<typename T>
void ProjectMultiSetting<T>::createSettingIfMissing(const QString& subKey)
{
	if (!settings.contains(subKey)) {
		settings.insert(subKey, new ProjectSetting<T>(table, baseKey + "/" + subKey, defaultValue));
	}
}



// List used types as compiler hints
template class ProjectMultiSetting<int>;
template class ProjectMultiSetting<bool>;
