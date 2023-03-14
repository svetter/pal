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
	
public:
	template<typename T>
	inline T readSetting(const ProjectSetting<T>* setting, int rowIndex = 0)
	{
		return setting->getValueAt(0);
	}
	template<typename T>
	inline T readSecondSetting(const ProjectSetting<T>* setting)
	{
		return readSetting(setting, 1);
	}
	
	template<typename T>
	inline void updateSetting(QWidget* parent, const ProjectSetting<T>* setting, QVariant value, int rowIndex = 0)
	{
		assert(rowIndex >= 0 && rowIndex < 2);
		
		ValidItemID primaryKey = primaryKeyColumn->getValueAt(rowIndex);
		updateCellInNormalTable(parent, primaryKey, setting, value);
	}
	template<typename T>
	inline void updateSecondSetting(QWidget* parent, const ProjectSetting<T>* setting, QVariant value)
	{
		return updateSetting(parent, setting, value, 1);
	}
};



#endif // SETTINGS_TABLE_H
