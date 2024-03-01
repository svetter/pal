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
 * @file ranges_table.h
 * 
 * This file defines the RangesTable class.
 */

#include "ranges_table.h"

#include "src/data/enum_names.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new RangesTable.
 */
RangesTable::RangesTable() :
	NormalTable(QString("Ranges"), tr("Mountain ranges"), "rangeID", tr("Mountain range ID")),
	//										name			uiName				type	nullable	enumNames
	nameColumn		(ValueColumn	(*this,	"name",			tr("Name"),			String,	false)),
	continentColumn	(ValueColumn	(*this,	"continent",	tr("Continent"),	Enum,	false,		&EnumNames::continentNames))
{
	addColumn(nameColumn);
	addColumn(continentColumn);
}



/**
 * Adds a new range to the table.
 *
 * @param parent	The parent widget.
 * @param range		The range to add.
 * @return			The index of the new range in the table buffer.
 */
BufferRowIndex RangesTable::addRow(QWidget& parent, Range& range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, range);
	
	BufferRowIndex newRangeIndex = NormalTable::addRow(parent, columnDataPairs);
	range.rangeID = getPrimaryKeyAt(newRangeIndex);
	return newRangeIndex;
}

/**
 * Updates the contents of an existing range in the table.
 *
 * @pre The given range's ItemID will be used to identify the range to update and thus must be valid.
 *
 * @param parent	The parent widget.
 * @param range		The range to update.
 */
void RangesTable::updateRow(QWidget& parent, ValidItemID rangeID, const Range& range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, range);
	
	NormalTable::updateRow(parent, rangeID, columnDataPairs);
}

/**
 * Updates the contents of existing ranges in the table.
 * 
 * @param parent		The parent widget.
 * @param rowIndices	The indices of the ranges to update in the table buffer.
 * @param columns		The columns to update. Can not contain the primary key column.
 * @param ascent		The new data for the ranges. The rangeID will be ignored.
 */
void RangesTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Range& range)
{
	assert(!columns.contains(&primaryKeyColumn));
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, range);
	
	NormalTable::updateRows(parent, rowIndices, columnDataPairs);
}


/**
 * Translates the data of a range to a list of column-data pairs.
 *
 * @param columns	The column list specifying the order of the data.
 * @param range		The range from which to get the data.
 * @return			A list of column-data pairs representing the range's data.
 */
const QList<ColumnDataPair> RangesTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Range& range) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &nameColumn)			{ data = range.name;		}
		else if (column == &continentColumn)	{ data = range.continent;	}
		else assert(false);
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns the translated string to be displayed to indicate that no range is selected.
 *
 * @return	The translated string representing absence of a mountain range.
 */
QString RangesTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns a translated message confirming that a new mountain range has been created.
 * 
 * @return	The translated message confirming creation of a new mountain range.
 */
QString RangesTable::getCreationConfirmMessage() const
{
	return tr("Saved new mountain range.");
}

/**
 * Returns a translated message confirming that a number of mountain ranges have been edited.
 *
 * @param numEdited	The number of mountain ranges that have been edited.
 * @return			The translated message confirming the editing of the mountain ranges.
 */
QString RangesTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln mountain ranges(s).", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of mountain ranges have been deleted.
 *
 * @param numDeleted	The number of mountain ranges that have been deleted.
 * @return				The translated message confirming the deletion of the mountain ranges.
 */
QString RangesTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln mountain ranges(s).", "", numDeleted);
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n mountain range(s)".
 * 
 * @param numItems	The number of mountain ranges to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString RangesTable::getItemCountString(int numItems) const
{
	return tr("%Ln mountain range(s)", "", numItems);
}
