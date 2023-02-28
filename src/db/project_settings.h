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
	
	inline void set(QWidget* parent, T value)
	{
		settingsTable->updateSetting(parent, this, value);
	}
};



class ProjectSettings : public SettingsTable {
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
};



#endif // PROJECT_SETTINGS_H
