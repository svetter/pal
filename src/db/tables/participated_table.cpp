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
 * @file participated_table.cpp
 * 
 * This file defines the ParticipatedTable class.
 */

#include "participated_table.h"

#include <QTranslator>



/**
 * Creates a new ParticipatedTable.
 * 
 * @param foreignAscentIDColumn	The primary key column of the AscentsTable.
 * @param foreignHikerIDColumn	The primary key column of the HikersTable.
 */
ParticipatedTable::ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn) :
		AssociativeTable(QString("Participated"), foreignAscentIDColumn, foreignHikerIDColumn),
		ascentIDColumn(getColumn1()),
		hikerIDColumn(getColumn2())
{}



/**
 * Adds one or more rows associating one more more hikers with an ascent to the table.
 * 
 * @param parent	The parent widget.
 * @param ascent	The ascent containing the hikers list to use.
 */
void ParticipatedTable::addRows(QWidget* parent, const Ascent* ascent)
{
	for (ValidItemID hikerID : ascent->hikerIDs) {
		QList<const Column*> columns = getColumnList();
		QList<QVariant> data = mapDataToQVariantList(columns, ascent->ascentID.forceValid(), hikerID);
		
		AssociativeTable::addRow(parent, columns, data);
	}
}

/**
 * Updates the table contents for a given ascent.
 * 
 * First removes all existing rows for the ascent, then adds rows for all hikers in the given
 * ascent object.
 * 
 * @param parent	The parent widget.
 * @param ascent	The ascent for which to update the table contents.
 */
void ParticipatedTable::updateRows(QWidget* parent, const Ascent* ascent)
{
	// delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascent->ascentID.forceValid());
	// add back all current rows
	addRows(parent, ascent);
}


/**
 * Translates the data of a single participation to a list of QVariants.
 *
 * @param columns	The column list specifying the order of the data.
 * @param ascentID	The ascentID of the participation.
 * @param hikerID	The hikerID of the participation.
 * @return			The list of QVariants representing the participation.
 */
QList<QVariant> ParticipatedTable::mapDataToQVariantList(QList<const Column*>& columns, ValidItemID ascentID, ValidItemID hikerID) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == ascentIDColumn)	{ data.append(ascentID.asQVariant());	continue; }
		if (column == hikerIDColumn)	{ data.append(hikerID.asQVariant());	continue; }
		assert(false);
	}
	return data;
}
