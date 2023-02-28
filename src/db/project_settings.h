#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include "src/db/table.h"
#include "src/db/settings_table.h"

#include <QString>
#include <QVariant>



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
	
	inline T get() const
	{
		return settingsTable->getBufferRow(0)->at(getIndex()).value<T>();
	}
	
	inline T getDefault() const
	{
		return defaultValue.value<T>();
	}
	
	inline void set(QWidget* parent, QVariant value) const
	{
		settingsTable->updateSetting(parent, this, value);
	}
	
	inline void setToNull(QWidget* parent) const
	{
		settingsTable->updateSetting(parent, this, QVariant());
	}
};



class ProjectSettings : public SettingsTable {
	// Can't store template objects ProjectSetting together directly, so storing Column + default value instead
	QList<QPair<Column*, QVariant>> defaults;
	
public:
	const ProjectSetting<int>*		defaultHiker;
	const ProjectSetting<bool>*		usePhotosBasePath;
	const ProjectSetting<QString>*	photosBasePath;
	
	inline ProjectSettings() :
			SettingsTable(),
			//												name					SQL type	nullable	table	default value
			defaultHiker		(new ProjectSetting<int>	("defaultHiker",		integer,	true,		this)),
			usePhotosBasePath	(new ProjectSetting<bool>	("usePhotosBasePath",	bit,		false,		this,	false)),
			photosBasePath		(new ProjectSetting<QString>("photosBasePath",		varchar,	true,		this))
	{
		addColumn(defaultHiker);
		addColumn(usePhotosBasePath);
		addColumn(photosBasePath);
	}
	
	template<typename T> inline void addSetting(ProjectSetting<T>* setting)
	{
		addColumn(setting);
		defaults.append({ (Column*) setting, setting->getDefault() });
	}
	
	inline void insertDefaults(QWidget* parent) {
		initBuffer(parent, true);
		
		QList<const Column*> columns = QList<const Column*>();
		QList<QVariant> values = QList<QVariant>();
		for (const QPair<Column*, QVariant> &columnDefaultPair : defaults) {
			columns.append(columnDefaultPair.first);
			values.append(columnDefaultPair.second);
		}
		addRow(parent, columns, values);
	}
};



#endif // PROJECT_SETTINGS_H
