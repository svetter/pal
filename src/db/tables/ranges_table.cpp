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
 * @file ranges_table.h
 * 
 * This file defines the RangesTable class.
 */

#include "ranges_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new RangesTable.
 */
RangesTable::RangesTable() :
		NormalTable(QString("Ranges"), tr("Mountain ranges"), "rangeID"),
		//							name			uiName				type	nullable	primaryKey	foreignKey	inTable
		nameColumn		(new Column("name",			tr("Name"),			String,	false,		false,		nullptr,	this)),
		continentColumn	(new Column("continent",	tr("Continent"),	Enum,	false,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
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
BufferRowIndex RangesTable::addRow(QWidget* parent, Range* range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, range);
	
	BufferRowIndex newRangeIndex = NormalTable::addRow(parent, columns, data);
	range->rangeID = getPrimaryKeyAt(newRangeIndex);
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
void RangesTable::updateRow(QWidget* parent, ValidItemID rangeID, const Range* range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, range);
	
	NormalTable::updateRow(parent, rangeID, columns, data);
}


/**
 * Translates the data of a range to a list of QVariants.
 *
 * @param columns	The column list specifying the order of the data.
 * @param range		The range from which to get the data.
 * @return			The list of QVariants representing the range's data.
 */
QList<QVariant> RangesTable::mapDataToQVariantList(QList<const Column*>& columns, const Range* range) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)		{ data.append(range->name);			continue; }
		if (column == continentColumn)	{ data.append(range->continent);	continue; }
		assert(false);
	}
	return data;
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
 * Returns the translation of "mountain range" (singular), not capitalized unless the language
 * requires it.
 *
 * @return	The translation of "mountain range" (singular) for use mid-sentence.
 */
QString RangesTable::getItemNameSingularLowercase() const
{
	return tr("mountain range");
}

/**
 * Returns the translation of "mountain ranges" (plural), not capitalized unless the language
 * requires it.
 *
 * @return	The translation of "mountain ranges" (plural) for use mid-sentence.
 */
QString RangesTable::getItemNamePluralLowercase() const
{
	return tr("mountain ranges");
}
