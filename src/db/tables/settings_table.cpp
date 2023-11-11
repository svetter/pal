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

/**
 * @file settings_table.cpp
 * 
 * This file defines the SettingsTable class.
 */

#include "settings_table.h"

#include "src/settings/project_settings.h"



/**
 * Creates a new SettingsTable.
 */
SettingsTable::SettingsTable() :
		Table("ProjectSettings", "Project settings", false),
		//								name				uiName		type	nullable	primaryKey	foreignKey	table
		primaryKeyColumn	(new Column("projectSettingID",	QString(),	ID,		false,		true,		nullptr,	this)),
		settingKeyColumn	(new Column("settingKey",		QString(),	String,	false,		false,		nullptr,	this)),
		settingValueColumn	(new Column("settingValue",		QString(),	String,	false,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(settingKeyColumn);
	addColumn(settingValueColumn);
}



/**
 * Indicates whether the given setting is present in the project settings table.
 * 
 * @param setting	The setting to check.
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			True if the setting is present, false otherwise.
 */
bool SettingsTable::settingIsPresent(const GenericProjectSetting* setting, QWidget* parent)
{
	return findSettingID(setting, parent).isValid();
}

/**
 * Returns the current value of the given setting in the project settings table.
 * 
 * If the parent window is given, the setting is added to the project settings table using the
 * default value if it is not present.
 * 
 * @param setting	The setting to look up.
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			The current value of the setting.
 */
QVariant SettingsTable::getSetting(const GenericProjectSetting* setting, QWidget* parent)
{
	ItemID id = findSettingID(setting);
	if (id.isInvalid()) {
		if (!parent) {
			return setting->defaultValue;
		}
		
		QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
		columnDataPairs.append({settingKeyColumn,	setting->key});
		columnDataPairs.append({settingValueColumn,	setting->defaultValue});
		BufferRowIndex newBufferIndex = addRow(parent, columnDataPairs);
		id = primaryKeyColumn->getValueAt(newBufferIndex);
	}
	
	return settingValueColumn->getValueFor(FORCE_VALID(id));
}


/**
 * Adds or updates the the setting in the project settings table with the given value.
 * 
 * @param parent	The parent window. Cannot be nullptr.
 * @param setting	The setting to add or update.
 * @param value		The new value for the setting.
 */
void SettingsTable::setSetting(QWidget* parent, const GenericProjectSetting* setting, QVariant value)
{
	ItemID id = findSettingID(setting, parent);
	if (id.isValid()) {
		// Update setting
		updateCellInNormalTable(parent, FORCE_VALID(id), settingValueColumn, value);
	} else {
		// Add setting
		QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
		columnDataPairs.append({settingKeyColumn,	setting->key});
		columnDataPairs.append({settingValueColumn,	value});
		addRow(parent, columnDataPairs);
	}
}

/**
 * Removes the setting from the project settings table entirely.
 * 
 * @param parent	The parent window. Cannot be nullptr.
 * @param setting	The setting to remove.
 */
void SettingsTable::removeSetting(QWidget* parent, const GenericProjectSetting* setting)
{
	ItemID id = findSettingID(setting, parent);
	if (id.isInvalid()) return;
	removeMatchingRows(parent, settingKeyColumn, FORCE_VALID(id));
}



/**
 * Finds the projectSettingID (primary key) of the given setting in the database.
 * 
 * @param setting	The setting to look up.
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			The projectSettingID of the setting, or an invalid ItemID if the setting is not present.
 */
ItemID SettingsTable::findSettingID(const GenericProjectSetting* setting, QWidget* parent)
{
	QList<BufferRowIndex> bufferRowIndices = getMatchingBufferRowIndices(settingKeyColumn, setting->key);
	
	if (bufferRowIndices.size() == 0) {
		return ItemID();
	}
	BufferRowIndex settingIndex = bufferRowIndices.last();
	
	if (bufferRowIndices.size() > 1) {
		QString error = "WARNING: Found " + QString::number(bufferRowIndices.size()) + " entries for project setting " + setting->key + ".";
		if (parent) {
			error += " Cleaning up.";
		}
		qDebug().noquote() << error;
		if (parent) {
			for (const BufferRowIndex& rowIndex : bufferRowIndices) {
				if (rowIndex == settingIndex) continue;	// Leave the last one in place
				ValidItemID id = VALID_ITEM_ID(primaryKeyColumn->getValueAt(rowIndex));
				removeMatchingRows(parent, settingKeyColumn, id);
			}
		}
	}
	
	return VALID_ITEM_ID(primaryKeyColumn->getValueAt(settingIndex));
}
