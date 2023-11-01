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

#include "regions_table.h"

#include <QString>
#include <QTranslator>



RegionsTable::RegionsTable(Column* foreignRangeIDColumn, Column* foreignCountryIDColumn) :
		NormalTable(QString("Regions"), tr("Regions"), "regionID"),
		//							name			uiName		type	nullable	primaryKey	foreignKey				inTable
		nameColumn		(new Column("name",			tr("Name"),	String,	false,		false,		nullptr,				this)),
		rangeIDColumn	(new Column("rangeID",		QString(),	ID,		true,		false,		foreignRangeIDColumn,	this)),
		countryIDColumn	(new Column("countryID",	QString(),	ID,		true,		false,		foreignCountryIDColumn,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
	addColumn(rangeIDColumn);
	addColumn(countryIDColumn);
}



BufferRowIndex RegionsTable::addRow(QWidget* parent, Region* region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, region);
	
	BufferRowIndex newRegionIndex = NormalTable::addRow(parent, columns, data);
	region->regionID = getPrimaryKeyAt(newRegionIndex);
	return newRegionIndex;
}

void RegionsTable::updateRow(QWidget* parent, ValidItemID regionID, const Region* region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, region);
	
	NormalTable::updateRow(parent, regionID, columns, data);
}


QList<QVariant> RegionsTable::mapDataToQVariantList(QList<const Column*>& columns, const Region* region) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)		{ data.append(region->name);					continue; }
		if (column == rangeIDColumn)	{ data.append(region->rangeID.asQVariant());	continue; }
		if (column == countryIDColumn)	{ data.append(region->countryID.asQVariant());	continue; }
		assert(false);
	}
	return data;
}



QString RegionsTable::getNoneString() const
{
	return tr("None");
}

QString RegionsTable::getItemNameSingularLowercase() const
{
	return tr("region");
}

QString RegionsTable::getItemNamePluralLowercase() const
{
	return tr("regions");
}
