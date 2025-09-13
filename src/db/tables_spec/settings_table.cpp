/*
 * Copyright 2023-2025 Simon Vetter
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
#include "src/db/database.h"



/**
 * Creates a new SettingsTable.
 * 
 * @param db	The database to which the table belongs.
 */
SettingsTable::SettingsTable(Database& db) :
	Table(db, "ProjectSettings", tr("Project settings"), false),
	//												name				uiName							type	nullable
	primaryKeyColumn	(PrimaryKeyColumn	(*this,	"projectSettingID",	tr("Project setting ID"))),
	settingKeyColumn	(ValueColumn		(*this,	"settingKey",		tr("Project setting key"),		String,	false)),
	settingValueColumn	(ValueColumn		(*this,	"settingValue",		tr("Project setting value"),	String,	true))
{
	addColumn(primaryKeyColumn);
	addColumn(settingKeyColumn);
	addColumn(settingValueColumn);
}



/**
 * Indicates whether the given setting is present and has a value in the project settings table.
 * 
 * @param setting	The setting to check.
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			True if the setting is present and not null, false otherwise.
 */
bool SettingsTable::settingIsPresent(const GenericProjectSetting& setting, QWidget* parent)
{
	const ItemID settingID = findSettingID(setting, parent);
	if (settingID.isInvalid()) return false;
	
	const QVariant value = settingValueColumn.getValueFor(FORCE_VALID(settingID));
	return value.isValid();
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
QVariant SettingsTable::getSetting(const GenericProjectSetting& setting, QWidget* parent)
{
	ItemID id = findSettingID(setting);
	if (id.isInvalid()) {
		if (!parent) {
			return setting.defaultValue;
		}
		
		QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
		columnDataPairs.append({&settingKeyColumn,		setting.key});
		columnDataPairs.append({&settingValueColumn,	setting.defaultValue});
		db.beginChangingData();
		BufferRowIndex newBufferIndex = addRow(*parent, columnDataPairs);
		db.finishChangingData();
		id = primaryKeyColumn.getValueAt(newBufferIndex);
	}
	
	return settingValueColumn.getValueFor(FORCE_VALID(id));
}


/**
 * Adds or updates the the setting in the project settings table with the given value.
 * 
 * @param parent	The parent window. Cannot be nullptr.
 * @param setting	The setting to add or update.
 * @param value		The new value for the setting.
 */
void SettingsTable::setSetting(QWidget& parent, const GenericProjectSetting& setting, QVariant value)
{
	const ItemID id = findSettingID(setting, &parent);
	if (id.isValid()) {
		// Update setting
		db.beginChangingData();
		updateCellInNormalTable(parent, FORCE_VALID(id), settingValueColumn, value);
		db.finishChangingData();
	} else {
		// Add setting
		QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
		columnDataPairs.append({&settingKeyColumn,		setting.key});
		columnDataPairs.append({&settingValueColumn,	value});
		
		db.beginChangingData();
		addRow(parent, columnDataPairs);
		db.finishChangingData();
	}
}

/**
 * Removes the value from the setting in the project settings table.
 * 
 * @param parent	The parent window. Cannot be nullptr.
 * @param setting	The setting to remove.
 */
void SettingsTable::clearSetting(QWidget& parent, const GenericProjectSetting& setting)
{
	const ItemID id = findSettingID(setting, &parent);
	if (id.isInvalid()) return;
	
	db.beginChangingData();
	updateCellInNormalTable(parent, FORCE_VALID(id), settingValueColumn, QVariant());
	db.finishChangingData();
}

/**
 * Removes the setting from the project settings table entirely.
 * 
 * @param parent	The parent window. Cannot be nullptr.
 * @param setting	The setting to remove.
 */
void SettingsTable::removeSetting(QWidget& parent, const GenericProjectSetting& setting)
{
	const ItemID id = findSettingID(setting, &parent);
	if (id.isInvalid()) return;
	
	db.beginChangingData();
	removeMatchingRows(parent, settingKeyColumn, FORCE_VALID(id));
	db.finishChangingData();
}


/**
 * Removes all project settings whose key starts with the given base key from the project settings
 * table.
 * 
 * @param parent	The parent window. Cannot be nullptr.
 * @param baseKey	The common key of all settings to remove.
 */
void SettingsTable::removeAllMatchingSettings(QWidget& parent, const QString& baseKey)
{
	db.beginChangingData();
	for (BufferRowIndex rowIndex = BufferRowIndex(buffer.numRows() - 1); rowIndex.isValid(); rowIndex--) {
		const QString key = settingKeyColumn.getValueAt(rowIndex).toString();
		if (key.startsWith(baseKey)) {
			const ValidItemID settingID = VALID_ITEM_ID(primaryKeyColumn.getValueAt(rowIndex));
			removeMatchingRows(parent, primaryKeyColumn, settingID);
		}
	}
	db.finishChangingData();
}



/**
 * Finds the projectSettingID (primary key) of the given setting in the database.
 * 
 * @param setting	The setting to look up.
 * @param parent	The parent window. Can be nullptr, in which case no cleanup is performed for duplicate settings.
 * @return			The projectSettingID of the setting, or an invalid ItemID if the setting is not present.
 */
ItemID SettingsTable::findSettingID(const GenericProjectSetting& setting, QWidget* parent)
{
	const QList<BufferRowIndex> bufferRowIndices = getMatchingBufferRowIndices(settingKeyColumn, setting.key);
	
	if (bufferRowIndices.size() == 0) {
		return ItemID();
	}
	BufferRowIndex settingIndex = bufferRowIndices.last();
	
	if (bufferRowIndices.size() > 1) {
		QString error = "WARNING: Found " + QString::number(bufferRowIndices.size()) + " entries for project setting " + setting.key + ".";
		if (parent) {
			error += " Cleaning up.";
		}
		qDebug().noquote() << error;
		if (parent) {
			for (const BufferRowIndex& rowIndex : bufferRowIndices) {
				if (rowIndex == settingIndex) continue;	// Leave the last one in place
				const ValidItemID id = VALID_ITEM_ID(primaryKeyColumn.getValueAt(rowIndex));
				db.beginChangingData();
				removeMatchingRows(*parent, settingKeyColumn, id);
				db.finishChangingData();
			}
		}
	}
	
	return VALID_ITEM_ID(primaryKeyColumn.getValueAt(settingIndex));
}
