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
 * @file project_settings.h
 * 
 * This file defines the ProjectSetting and ProjectSettings classes.
 */

#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include "src/db/table.h"
#include "src/db/settings_table.h"

#include <QString>
#include <QVariant>
#include <QDate>



/**
 * A class representing a project setting.
 */
template<typename T>
class ProjectSetting : public Column {
	/** The project settings table. */
	SettingsTable* settingsTable;
	/** The default value for the setting. */
	const QVariant defaultValue;
	
public:
	/**
	 * Creates a new ProjectSetting.
	 * 
	 * @param key			The key of the setting.
	 * @param type			The data type of the setting.
	 * @param nullable		Whether the setting is nullable.
	 * @param settingsTable	The project settings table.
	 * @param defaultValue	The default value for the setting.
	 */
	inline ProjectSetting(const QString key, DataType type, bool nullable, SettingsTable* settingsTable, QVariant defaultValue = QVariant()) :
			Column(key, QString(), type, nullable, false, nullptr, settingsTable),
			settingsTable(settingsTable),
			defaultValue(defaultValue)
	{}
	
	/**
	 * Returns the default value for the setting.
	 * 
	 * @return	The default value for the setting.
	 */
	inline QVariant getDefault() const
	{
		return defaultValue;
	}
	
	/**
	 * Returns whether the setting is null at the given row index (0 or 1, 0 by default).
	 * 
	 * @param rowIndex	The row index of the setting.
	 * @return			Whether the setting is null.
	 */
	inline bool isNotNull(int rowIndex = 0) const
	{
		assert(rowIndex < 2);
		QVariant value = getValueAt(BufferRowIndex(rowIndex));
		return value.isValid() && !value.isNull();
	}

	/**
	 * Returns whether the setting is null at the second row index (1).
	 * 
	 * @return	Whether the setting is null at the second row index.
	 */
	inline bool secondIsNotNull() const
	{
		return isNotNull(1);
	}
	
	/**
	 * Returns the current value of the setting at the given row index (0 or 1, 0 by default).
	 * 
	 * @param rowIndex	The row index of the setting.
	 * @return			The current value of the setting.
	 */
	inline T get(int rowIndex = 0) const
	{
		assert(rowIndex < 2);
		return getValueAt(BufferRowIndex(rowIndex)).template value<T>();
	}

	/**
	 * Returns the current value of the setting at the second row index (1).
	 * 
	 * @return	The current value of the setting at the second row index.
	 */
	inline T getSecond() const
	{
		return get(1);
	}
	
	/**
	 * Sets the value of the setting at the given row index (0 or 1, 0 by default).
	 * 
	 * @param parent	The parent window.
	 * @param value		The new value for the setting.
	 * @param rowIndex	The row index of the setting.
	 */
	inline void set(QWidget* parent, QVariant value, int rowIndex = 0) const
	{
		assert(rowIndex >= 0 && rowIndex < 2);
		settingsTable->updateSetting(parent, this, value, rowIndex);
	}

	/**
	 * Sets the value of the setting at the second row index (1).
	 * 
	 * @param parent	The parent window.
	 * @param value		The new value for the setting.
	 */
	inline void setSecond(QWidget* parent, QVariant value) const
	{
		return set(parent, value, 1);
	}
	
	/**
	 * Sets the value of the setting to null at the given row index (0 or 1, 0 by default).
	 * 
	 * @param parent	The parent window.
	 * @param rowIndex	The row index of the setting.
	 */
	inline void setToNull(QWidget* parent, int rowIndex = 0) const
	{
		set(parent, QVariant(), rowIndex);
	}

	/**
	 * Sets the value of the setting to null at the second row index (1).
	 * 
	 * @param parent	The parent window.
	 */
	inline void setSecondToNull(QWidget* parent) const
	{
		setToNull(parent, 1);
	}

	/**
	 * Sets the value of the setting to null at both row indices (0 and 1).
	 * 
	 * @param parent	The parent window.
	 */
	inline void setBothToNull(QWidget* parent) const
	{
		setToNull(parent);
		setSecondToNull(parent);
	}
};



/**
 * A class for managing all project settings.
 */
class ProjectSettings : public SettingsTable {
	// Can't store template objects ProjectSetting together directly, so storing Column + default value instead
	/**
	 * The list of all project settings and their default values.
	 */
	QList<QPair<Column*, QVariant>> defaults;
	
public:
	/** The default hiker setting. */
	const ProjectSetting<int>*		defaultHiker;
	// Implicit settings
	/** The date filter setting. */
	const ProjectSetting<QDate>*	dateFilter;
	/** The peak height filter setting. */
	const ProjectSetting<int>*		peakHeightFilter;
	/** The volcano filter setting. */
	const ProjectSetting<bool>*		volcanoFilter;
	/** The range filter setting. */
	const ProjectSetting<int>*		rangeFilter;
	/** The hike kind filter setting. */
	const ProjectSetting<int>*		hikeKindFilter;
	/** The difficulty filter setting. */
	const ProjectSetting<int>*		difficultyFilter;
	/** The hiker filter setting. */
	const ProjectSetting<int>*		hikerFilter;
	
	/**
	 * Adds a setting to the project settings table.
	 * 
	 * Only used during initialization.
	 * 
	 * @param setting	The setting to add.
	 */
	template<typename T> inline void addSetting(const ProjectSetting<T>* setting)
	{
		addColumn(setting);
		defaults.append({ (Column*) setting, setting->getDefault() });
	}
	
	/**
	 * Creates a new ProjectSettings object.
	 */
	inline ProjectSettings() :
			SettingsTable(),
			//												name					SQL type	nullable	table	default value
			defaultHiker		(new ProjectSetting<int>	("defaultHiker",		ID,			true,		this)),
			// Implicit settings
			dateFilter			(new ProjectSetting<QDate>	("dateFilter",			Date,		true,		this)),
			peakHeightFilter	(new ProjectSetting<int>	("peakHeightFilter",	Integer,	true,		this)),
			volcanoFilter		(new ProjectSetting<bool>	("volcanoFilter",		Bit,		true,		this)),
			rangeFilter			(new ProjectSetting<int>	("rangeFilter",			ID,			true,		this)),
			hikeKindFilter		(new ProjectSetting<int>	("hikeKindFilter",		Enum,		true,		this)),
			difficultyFilter	(new ProjectSetting<int>	("difficultyFilter",	DualEnum,	true,		this)),
			hikerFilter			(new ProjectSetting<int>	("hikerFilter",			ID,			true,		this))
	{
		addSetting(defaultHiker);
		// Implicit settings
		addSetting(dateFilter);
		addSetting(peakHeightFilter);
		addSetting(volcanoFilter);
		addSetting(rangeFilter);
		addSetting(hikeKindFilter);
		addSetting(difficultyFilter);
		addSetting(hikerFilter);
	}
	
	/**
	 * Sets all settings to their default values.
	 * 
	 * @param setting	The setting.
	 * @return			The default value for the setting.
	 */
	inline void insertDefaults(QWidget* parent) {
		QList<const Column*> columns = QList<const Column*>();
		QList<QVariant> values = QList<QVariant>();
		QList<QVariant> secondRowValues = QList<QVariant>();
		for (const QPair<Column*, QVariant> &columnDefaultPair : defaults) {
			columns.append(columnDefaultPair.first);
			values.append(columnDefaultPair.second);
			secondRowValues.append(QVariant());
		}
		addRow(parent, columns, values);
		addRow(parent, columns, secondRowValues);
	}
};



#endif // PROJECT_SETTINGS_H
