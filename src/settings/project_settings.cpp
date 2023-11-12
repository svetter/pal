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
 * This file defines the ProjectSetting class.
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
bool GenericProjectSetting::isPresent(QWidget* parent)
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
QVariant GenericProjectSetting::getAsQVariant(QWidget* parent)
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
void GenericProjectSetting::set(QWidget* parent, QVariant value)
{
	table->setSetting(parent, this, value);
}

/**
 * Clears the setting in the project settings storage.
 *
 * @param parent	The parent window. Cannot be nullptr.
 */
void GenericProjectSetting::clear(QWidget* parent)
{
	table->clearSetting(parent, this);
}

/**
 * Removes the setting from the project settings storage completely.
 *
 * @param parent	The parent window. Cannot be nullptr.
 */
void GenericProjectSetting::remove(QWidget* parent)
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
T ProjectSetting<T>::get(QWidget* parent)
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
