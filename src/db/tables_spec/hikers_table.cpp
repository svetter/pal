/*
 * Copyright 2023-2024 Simon Vetter
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
 * @file hikers_table.h
 * 
 * This file defines the HikersTable class.
 */

#include "hikers_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new HikersTable.
 * 
 * @param db	The database to which the table belongs.
 */
HikersTable::HikersTable(Database& db) :
	NormalTable(db, QString("Hikers"), tr("Hikers"), "hikerID", tr("Hiker ID")),
	//									name	uiName		type	nullable
	nameColumn	(ValueColumn	(*this,	"name",	tr("Name"),	String,	false))
{
	addColumn(nameColumn);
}



/**
 * Adds a new hiker to the table.
 * 
 * @param parent	The parent widget.
 * @param hiker	The hiker to add.
 * @return			The index of the new hiker in the table buffer.
 */
BufferRowIndex HikersTable::addRow(QWidget& parent, Hiker& hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, hiker);
	
	BufferRowIndex newHikerIndex = NormalTable::addRow(parent, columnDataPairs);
	hiker.hikerID = getPrimaryKeyAt(newHikerIndex);
	return newHikerIndex;
}

/**
 * Updates the contents of an existing hiker in the table.
 * 
 * @param parent	The parent widget.
 * @param hikerID	The ID of the hiker to update.
 * @param hiker		The hiker data to write to the row with the given ID.
 */
void HikersTable::updateRow(QWidget& parent, ValidItemID hikerID, const Hiker& hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, hiker);
	
	NormalTable::updateRow(parent, hikerID, columnDataPairs);
}

/**
 * Updates the contents of existing hikers in the table.
 * 
 * @param parent		The parent widget.
 * @param rowIndices	The indices of the hikers to update in the table buffer.
 * @param columns		The columns to update. Can not contain the primary key column.
 * @param hiker			The new data for the hikers. The hikerID will be ignored.
 */
void HikersTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Hiker& hiker)
{
	assert(!columns.contains(&primaryKeyColumn));
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, hiker);
	
	NormalTable::updateRows(parent, rowIndices, columnDataPairs);
}


/**
 * Translates the data of a hiker to a list of column-data pairs.
 * 
 * @param columns	The column list specifying the order of the data.
 * @param hiker		The hiker from which to get the data.
 * @return			A list of column-data pairs representing the hiker's data.
 */
const QList<ColumnDataPair> HikersTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Hiker& hiker) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &nameColumn)	{ data = hiker.name;	}
		else assert(false);
		
		if (column->type == String && data.toString().isEmpty()) data = QVariant();
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns a string representation of the hiker at the given buffer row index.
 * 
 * @param bufferRow	The buffer row index of the hiker to represent.
 * @return			A UI-appropriate string representation of the hiker.
 */
QString HikersTable::getIdentityRepresentationAt(const BufferRowIndex& bufferRow) const
{
	return nameColumn.getValueAt(bufferRow).toString();
}

/**
 * Returns a list of all columns used for identity representation of hikers.
 * 
 * @return	A list of all columns used for identity representation.
 */
QList<const Column*> HikersTable::getIdentityRepresentationColumns() const
{
	return { &nameColumn };
}



/**
 * Returns the translation of "Hiker" (singular).
 * 
 * @return	The translation of "Hiker" (singular).
 */
QString HikersTable::getItemNameSingular() const
{
	return tr("Hiker");
}

/**
 * Returns the translated string to be displayed to indicate that no hiker is selected.
 * 
 * @return	The translated string representing absence of a hiker.
 */
QString HikersTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns a translated message confirming that a new hiker has been created.
 * 
 * @return	The translated message confirming creation of a new hiker.
 */
QString HikersTable::getCreationConfirmMessage() const
{
	return tr("Saved new hiker.");
}

/**
 * Returns a translated message confirming that a number of hikers have been edited.
 * 
 * @param numEdited	The number of hikers that have been edited.
 * @return			The translated message confirming the editing of the hikers.
 */
QString HikersTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln hiker(s).", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of hikers have been deleted.
 * 
 * @param numDeleted	The number of hikers that have been deleted.
 * @return				The translated message confirming the deletion of the hikers.
 */
QString HikersTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln hiker(s).", "", numDeleted);
}

/**
 * Returns a translated title for a hiker custom column wizard.
 * 
 * @return	The translated title for a hiker custom column wizard.
 */
QString HikersTable::getNewCustomColumnString() const
{
	return tr("New custom column hikers");
}

/**
 * Returns a translated title for a hiker filter wizard.
 * 
 * @return	The translated title for a hiker filter wizard.
 */
QString HikersTable::getNewFilterString() const
{
	return tr("New hiker filter");
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n hiker(s)".
 * 
 * @param numItems	The number of hikers to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString HikersTable::getItemCountString(int numItems) const
{
	return tr("%Ln hiker(s)", "", numItems);
}
