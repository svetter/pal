#include "settings_table.h"

#include <QString>
#include <QVariant>



SettingsTable::SettingsTable() :
		Table("ProjectSettings", "Project settings", false),
		primaryKeyColumn	(new Column("projectSettingsID", QString(), integer, false, true, nullptr, this))
{
	addColumn(primaryKeyColumn);
}



void SettingsTable::initializeForNewDatabase(QWidget* parent) {
	createTableInSql(parent);
	initBuffer(parent, true);
	
	QList<const Column*> columns = QList<const Column*>();
	QList<QVariant> values = QList<QVariant>();
	for (const ProjectSetting* setting : settings) {
		columns.append((Column*) setting);
		values.append(setting->getDefault());
	}
	addRow(parent, columns, values);
}



void SettingsTable::initializeForExistingDatabase(QWidget* parent) {
	initBuffer(parent);
}

QVariant SettingsTable::readSetting(const ProjectSetting* setting)
{
	// TODO
}

void SettingsTable::updateSetting(QWidget* parent, const ProjectSetting* setting, QVariant value)
{
	// TODO
}
