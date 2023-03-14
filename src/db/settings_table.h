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
		assert(rowIndex >= 0 && rowIndex < 2);
		
		ValidItemID primaryKey = primaryKeyColumn->getValueAt(rowIndex);
		updateCellInNormalTable(parent, primaryKey, setting, value);
	}
	
	template<typename T> friend class ProjectSetting;
};



#endif // SETTINGS_TABLE_H
