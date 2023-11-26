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
 * @file hikers_table.h
 * 
 * This file defines the HikersTable class.
 */

#include "hikers_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new HikersTable.
 */
HikersTable::HikersTable() :
	NormalTable(QString("Hikers"), tr("Hikers"), "hikerID", tr("Hiker ID")),
	//										name	uiName		type	nullable
	nameColumn	(new ValueColumn	(this,	"name",	tr("Name"),	String,	false))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
}



/**
 * Adds a new hiker to the table.
 *
 * @param parent	The parent widget.
 * @param hiker	The hiker to add.
 * @return			The index of the new hiker in the table buffer.
 */
BufferRowIndex HikersTable::addRow(QWidget* parent, Hiker* hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, hiker);
	
	BufferRowIndex newHikerIndex = NormalTable::addRow(parent, columnDataPairs);
	hiker->hikerID = getPrimaryKeyAt(newHikerIndex);
	return newHikerIndex;
}

/**
 * Updates the contents of an existing hiker in the table.
 *
 * @pre The given hiker's ItemID will be used to identify the hiker to update and thus must be valid.
 *
 * @param parent	The parent widget.
 * @param hiker		The hiker to update.
 */
void HikersTable::updateRow(QWidget* parent, ValidItemID hikerID, const Hiker* hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, hiker);
	
	NormalTable::updateRow(parent, hikerID, columnDataPairs);
}


/**
 * Translates the data of a hiker to a list of column-data pairs.
 *
 * @param columns	The column list specifying the order of the data.
 * @param hiker		The hiker from which to get the data.
 * @return			A list of column-data pairs representing the hiker's data.
 */
const QList<ColumnDataPair> HikersTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Hiker* hiker) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == nameColumn)	{ data = hiker->name;	}
		else assert(false);
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
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
 * Returns the translation of "hiker" (singular), not capitalized unless the language requires it.
 *
 * @return	The translation of "hiker" (singular) for use mid-sentence.
 */
QString HikersTable::getItemNameSingularLowercase() const
{
	return tr("hiker");
}

/**
 * Returns the translation of "hikers" (plural), not capitalized unless the language requires it.
 *
 * @return	The translation of "hikers" (plural) for use mid-sentence.
 */
QString HikersTable::getItemNamePluralLowercase() const
{
	return tr("hikers");
}
