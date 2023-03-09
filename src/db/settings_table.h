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
			primaryKeyColumn	(new Column("projectSettingsID", QString(), integer, false, true, nullptr, this))
	{
		addColumn(primaryKeyColumn);
	}
	
public:
	template<typename T>
	inline T readSetting(const ProjectSetting<T>* setting)
	{
		return setting->getValueAt(0);
	}
	
	template<typename T>
	inline void updateSetting(QWidget* parent, const ProjectSetting<T>* setting, QVariant value)
	{
		ValidItemID primaryKey = primaryKeyColumn->getValueAt(0);
		updateCellInNormalTable(parent, primaryKey, setting, value);
	}
};



#endif // SETTINGS_TABLE_H
