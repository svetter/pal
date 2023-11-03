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
 * @file regions_table.h
 * 
 * This file defines the RegionsTable class.
 */

#include "regions_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new RegionsTable.
 * 
 * @param foreignRangeIDColumn		The primary key column of the RangesTable.
 * @param foreignCountryIDColumn	The primary key column of the CountriesTable.
 */
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



/**
 * Adds a new region to the table.
 *
 * @param parent	The parent widget.
 * @param region	The region to add.
 * @return			The index of the new region in the table buffer.
 */
BufferRowIndex RegionsTable::addRow(QWidget* parent, Region* region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, region);
	
	BufferRowIndex newRegionIndex = NormalTable::addRow(parent, columns, data);
	region->regionID = getPrimaryKeyAt(newRegionIndex);
	return newRegionIndex;
}

/**
 * Updates the contents of an existing region in the table.
 *
 * @pre The given region's ItemID will be used to identify the region to update and thus must be valid.
 *
 * @param parent	The parent widget.
 * @param region	The region to update.
 */
void RegionsTable::updateRow(QWidget* parent, ValidItemID regionID, const Region* region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, region);
	
	NormalTable::updateRow(parent, regionID, columns, data);
}


/**
 * Translates the data of a region to a list of QVariants.
 *
 * @param columns	The column list specifying the order of the data.
 * @param region	The region from which to get the data.
 * @return			The list of QVariants representing the region's data.
 */
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



/**
 * Returns the translated string to be displayed to indicate that no region is selected.
 *
 * @return	The translated string representing absence of a region.
 */
QString RegionsTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns the translation of "region" (singular), not capitalized unless the language requires it.
 *
 * @return	The translation of "region" (singular) for use mid-sentence.
 */
QString RegionsTable::getItemNameSingularLowercase() const
{
	return tr("region");
}

/**
 * Returns the translation of "regions" (plural), not capitalized unless the language requires it.
 *
 * @return	The translation of "regions" (plural) for use mid-sentence.
 */
QString RegionsTable::getItemNamePluralLowercase() const
{
	return tr("regions");
}
