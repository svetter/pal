#ifndef SETTINGS_TABLE_H
#define SETTINGS_TABLE_H

#include "src/db/table.h"

#include <QWidget>
#include <QVariant>

class ProjectSetting;



class SettingsTable : public Table {
	Column* primaryKeyColumn;
	
public:
	SettingsTable();
	
public:
	void initializeForNewDatabase(QWidget* parent);
	void initializeForExistingDatabase(QWidget* parent);
	
	QVariant readSetting(const ProjectSetting* setting);
	void updateSetting(QWidget* parent, const ProjectSetting* setting, QVariant value);
};



#endif // SETTINGS_TABLE_H
