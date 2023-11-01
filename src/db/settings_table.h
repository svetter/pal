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

#ifndef SETTINGS_TABLE_H
#define SETTINGS_TABLE_H

#include "src/db/table.h"

#include <QWidget>
#include <QVariant>

template<typename T> class ProjectSetting;



class SettingsTable : public Table {
	Column* primaryKeyColumn;
	
public:
	inline SettingsTable() :
			Table("ProjectSettings", "Project settings", false),
			primaryKeyColumn	(new Column("projectSettingsID", QString(), ID, false, true, nullptr, this))
	{
		addColumn(primaryKeyColumn);
	}
	
protected:
	template<typename T>
	inline void updateSetting(QWidget* parent, const ProjectSetting<T>* setting, QVariant value, int rowIndex = 0)
	{
		assert(rowIndex < 2);
		
		ValidItemID primaryKey = primaryKeyColumn->getValueAt(BufferRowIndex(rowIndex));
		updateCellInNormalTable(parent, primaryKey, setting, value);
	}
	
	template<typename T> friend class ProjectSetting;
};



#endif // SETTINGS_TABLE_H
