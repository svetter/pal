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

#include "ranges_table.h"

#include <QString>
#include <QTranslator>



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



BufferRowIndex RangesTable::addRow(QWidget* parent, Range* range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, range);
	
	BufferRowIndex newRangeIndex = NormalTable::addRow(parent, columns, data);
	range->rangeID = getPrimaryKeyAt(newRangeIndex);
	return newRangeIndex;
}

void RangesTable::updateRow(QWidget* parent, ValidItemID rangeID, const Range* range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, range);
	
	NormalTable::updateRow(parent, rangeID, columns, data);
}


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



QString RangesTable::getNoneString() const
{
	return tr("None");
}

QString RangesTable::getItemNameSingularLowercase() const
{
	return tr("mountain range");
}

QString RangesTable::getItemNamePluralLowercase() const
{
	return tr("mountain ranges");
}
