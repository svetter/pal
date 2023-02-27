#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include "qsqldatabase.h"
#include "src/db/table.h"

#include <QString>
#include <QVariant>



class ProjectSetting : public Column {
	const QVariant defaultValue;
	
public:
	inline ProjectSetting(const QString key, DataType type, bool nullable, Table* settingsTable, QVariant defaultValue = QVariant()) :
			Column(key, QString(), type, nullable, false, nullptr, settingsTable),
			defaultValue(defaultValue)
	{}
	
	inline QVariant getDefault() const
	{
		return defaultValue;
	}
};



class ProjectSettings : public Table {
	QList<const ProjectSetting*> settings;
	Column* primaryKeyColumn;
	
public:
	const ProjectSetting* defaultHiker;
	const ProjectSetting* usePhotosBasePath;
	const ProjectSetting* photosBasePath;
	
	inline ProjectSettings() :
			Table("ProjectSettings", "Project settings", false),
			primaryKeyColumn	(new Column("projectSettingsID", QString(), integer, false, true, nullptr, this)),
			//										name					SQL type	nullable	table		default value
			defaultHiker		(new ProjectSetting("defaultHiker",			integer,	true,		this)),
			usePhotosBasePath	(new ProjectSetting("usePhotosBasePath",	bit,		false,		this,		false)),
			photosBasePath		(new ProjectSetting("photosBasePath",		varchar,	true,		this))
	{
		addColumn(primaryKeyColumn);
		addSetting(defaultHiker);
		addSetting(usePhotosBasePath);
		addSetting(photosBasePath);
	}
	
private:
	inline void addSetting(const ProjectSetting* setting)
	{
		addColumn(setting);
		settings.append(setting);
	}
	
public:
	inline void initializeForNewDatabase(QWidget* parent) {
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
	
	inline void initializeForExistingDatabase(QWidget* parent) {
		initBuffer(parent);
	}
	
	
	inline QVariant getValue(const ProjectSetting* setting) const
	{
		return getBufferRow(0)->at(setting->getIndex());
	}
	
	inline void setValue(QWidget* parent, ProjectSetting* setting, QVariant value)
	{
		ValidItemID primaryKey = getBufferRow(0)->at(primaryKeyColumn->getIndex()).toInt();
		updateCellInNormalTable(parent, primaryKey, setting, value);
	}
};



#endif // PROJECT_SETTINGS_H
