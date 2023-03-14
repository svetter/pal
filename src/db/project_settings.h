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
	// Implicit settings
	const ProjectSetting<QDate>*	dateFilter;
	const ProjectSetting<int>*		peakHeightFilter;
	const ProjectSetting<bool>*		volcanoFilter;
	const ProjectSetting<int>*		rangeFilter;
	const ProjectSetting<int>*		hikeKindFilter;
	const ProjectSetting<int>*		difficultyFilterSystem;
	const ProjectSetting<int>*		difficultyFilterGrade;
	const ProjectSetting<int>*		hikerFilter;
	
	template<typename T> inline void addSetting(const ProjectSetting<T>* setting)
	{
		addColumn(setting);
		defaults.append({ (Column*) setting, setting->getDefault() });
	}
	
	inline ProjectSettings() :
			SettingsTable(),
			//													name						SQL type	nullable	table	default value
			defaultHiker			(new ProjectSetting<int>	("defaultHiker",			ID,			true,		this)),
			// Implicit settings
			dateFilter				(new ProjectSetting<QDate>	("dateFilter",				Date,		true,		this)),
			peakHeightFilter		(new ProjectSetting<int>	("peakHeightFilter",		Integer,	true,		this)),
			volcanoFilter			(new ProjectSetting<bool>	("volcanoFilter",			Bit,		true,		this)),
			rangeFilter				(new ProjectSetting<int>	("rangeFilter",				ID,			true,		this)),
			hikeKindFilter			(new ProjectSetting<int>	("hikeKindFilter",			Enum,		true,		this)),
			difficultyFilterSystem	(new ProjectSetting<int>	("difficultyFilterSystem",	DualEnum,	true,		this)),
			difficultyFilterGrade	(new ProjectSetting<int>	("difficultyFilterGrade",	DualEnum,	true,		this)),
			hikerFilter				(new ProjectSetting<int>	("hikerFilter",				ID,			true,		this))
	{
		addSetting(defaultHiker);
		// Implicit settings
		addSetting(dateFilter);
		addSetting(peakHeightFilter);
		addSetting(volcanoFilter);
		addSetting(rangeFilter);
		addSetting(hikeKindFilter);
		addSetting(difficultyFilterSystem);
		addSetting(difficultyFilterGrade);
		addSetting(hikerFilter);
	}
	
	inline void insertDefaults(QWidget* parent) {
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
