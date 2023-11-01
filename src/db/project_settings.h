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

#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include "src/db/table.h"
#include "src/db/settings_table.h"

#include <QString>
#include <QVariant>
#include <QDate>



template<typename T>
class ProjectSetting : public Column {
	SettingsTable* settingsTable;
	const QVariant defaultValue;
	
public:
	inline ProjectSetting(const QString key, DataType type, bool nullable, SettingsTable* settingsTable, QVariant defaultValue = QVariant()) :
			Column(key, QString(), type, nullable, false, nullptr, settingsTable),
			settingsTable(settingsTable),
			defaultValue(defaultValue)
	{}
	
	inline QVariant getDefault() const
	{
		return defaultValue;
	}
	
	inline bool isNotNull(int rowIndex = 0) const
	{
		assert(rowIndex < 2);
		QVariant value = getValueAt(BufferRowIndex(rowIndex));
		return value.isValid() && !value.isNull();
	}
	inline bool secondIsNotNull() const
	{
		return isNotNull(1);
	}
	
	inline T get(int rowIndex = 0) const
	{
		assert(rowIndex < 2);
		return getValueAt(BufferRowIndex(rowIndex)).template value<T>();
	}
	inline T getSecond() const
	{
		return get(1);
	}
	
	inline void set(QWidget* parent, QVariant value, int rowIndex = 0) const
	{
		assert(rowIndex >= 0 && rowIndex < 2);
		settingsTable->updateSetting(parent, this, value, rowIndex);
	}
	inline void setSecond(QWidget* parent, QVariant value) const
	{
		return set(parent, value, 1);
	}
	
	inline void setToNull(QWidget* parent, int rowIndex = 0) const
	{
		set(parent, QVariant(), rowIndex);
	}
	inline void setSecondToNull(QWidget* parent) const
	{
		setToNull(parent, 1);
	}
	inline void setBothToNull(QWidget* parent) const
	{
		setToNull(parent);
		setSecondToNull(parent);
	}
};



class ProjectSettings : public SettingsTable {
	// Can't store template objects ProjectSetting together directly, so storing Column + default value instead
	QList<QPair<Column*, QVariant>> defaults;
	
public:
	const ProjectSetting<int>*		defaultHiker;
	// Implicit settings
	const ProjectSetting<QDate>*	dateFilter;
	const ProjectSetting<int>*		peakHeightFilter;
	const ProjectSetting<bool>*		volcanoFilter;
	const ProjectSetting<int>*		rangeFilter;
	const ProjectSetting<int>*		hikeKindFilter;
	const ProjectSetting<int>*		difficultyFilter;
	const ProjectSetting<int>*		hikerFilter;
	
	template<typename T> inline void addSetting(const ProjectSetting<T>* setting)
	{
		addColumn(setting);
		defaults.append({ (Column*) setting, setting->getDefault() });
	}
	
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
