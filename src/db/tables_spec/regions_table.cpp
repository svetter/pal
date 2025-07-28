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
 * @param db						The database to which the table belongs.
 * @param foreignRangeIDColumn		The primary key column of the RangesTable.
 * @param foreignCountryIDColumn	The primary key column of the CountriesTable.
 */
RegionsTable::RegionsTable(Database& db, PrimaryKeyColumn& foreignRangeIDColumn, PrimaryKeyColumn& foreignCountryIDColumn) :
	NormalTable(db, QString("Regions"), tr("Regions"), "regionID", tr("Region ID")),
	//											name			uiName						type	nullable	foreignColumn
	nameColumn		(ValueColumn		(*this,	"name",			tr("Name"),					String,	false)),
	rangeIDColumn	(ForeignKeyColumn	(*this,	"rangeID",		tr("Mountain range ID"),			true,		foreignRangeIDColumn)),
	countryIDColumn	(ForeignKeyColumn	(*this,	"countryID",	tr("Country ID"),					true,		foreignCountryIDColumn))
{
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
BufferRowIndex RegionsTable::addRow(QWidget& parent, Region& region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, region);
	
	BufferRowIndex newRegionIndex = NormalTable::addRow(parent, columnDataPairs);
	region.regionID = getPrimaryKeyAt(newRegionIndex);
	return newRegionIndex;
}

/**
 * Updates the contents of an existing region in the table.
 * 
 * @param parent	The parent widget.
 * @param regionID	The ID of the region to update.
 * @param region	The region data to write to the row with the given ID.
 */
void RegionsTable::updateRow(QWidget& parent, ValidItemID regionID, const Region& region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, region);
	
	NormalTable::updateRow(parent, regionID, columnDataPairs);
}

/**
 * Updates the contents of existing regions in the table.
 * 
 * @param parent		The parent widget.
 * @param rowIndices	The indices of the regions to update in the table buffer.
 * @param columns		The columns to update. Can not contain the primary key column.
 * @param region		The new data for the regions. The regionID will be ignored.
 */
void RegionsTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Region& region)
{
	assert(!columns.contains(&primaryKeyColumn));
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, region);
	
	NormalTable::updateRows(parent, rowIndices, columnDataPairs);
}


/**
 * Translates the data of a region to a list of column-data pairs.
 * 
 * @param columns	The column list specifying the order of the data.
 * @param region	The region from which to get the data.
 * @return			A list of column-data pairs representing the region's data.
 */
const QList<ColumnDataPair> RegionsTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Region& region) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &nameColumn)			{ data = region.name;					}
		else if (column == &rangeIDColumn)		{ data = region.rangeID.asQVariant();	}
		else if (column == &countryIDColumn)	{ data = region.countryID.asQVariant();	}
		else assert(false);
		
		if (column->type == String && data.toString().isEmpty()) data = QVariant();
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns a string representation of the region at the given buffer row index.
 * 
 * @param bufferRow	The buffer row index of the region to represent.
 * @return			A UI-appropriate string representation of the region.
 */
QString RegionsTable::getIdentityRepresentationAt(const BufferRowIndex& bufferRow) const
{
	return nameColumn.getValueAt(bufferRow).toString();
}

/**
 * Returns a list of all columns used for identity representation of regions.
 * 
 * @return	A list of all columns used for identity representation.
 */
QList<const Column*> RegionsTable::getIdentityRepresentationColumns() const
{
	return { &nameColumn };
}



/**
 * Returns the translation of "Region" (singular).
 * 
 * @return	The translation of "Region" (singular).
 */
QString RegionsTable::getItemNameSingular() const
{
	return tr("Region");
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
 * Returns a translated prompt to edit an existing region.
 * 
 * @return	The translated prompt to edit an existing region.
 */
QString RegionsTable::getEditItemString() const
{
	return tr("Edit region...");
}

/**
 * Returns a translated message confirming that a new region has been created.
 * 
 * @return	The translated message confirming creation of a new region.
 */
QString RegionsTable::getCreationConfirmMessage() const
{
	return tr("Saved new region.");
}

/**
 * Returns a translated message confirming that a number of regions have been edited.
 * 
 * @param numEdited	The number of regions that have been edited.
 * @return			The translated message confirming the editing of the regions.
 */
QString RegionsTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln region(s).", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of regions have been deleted.
 * 
 * @param numDeleted	The number of regions that have been deleted.
 * @return				The translated message confirming the deletion of the regions.
 */
QString RegionsTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln region(s).", "", numDeleted);
}

/**
 * Returns a translated title for a region custom column wizard.
 * 
 * @return	The translated title for a region custom column wizard.
 */
QString RegionsTable::getNewCustomColumnString() const
{
	return tr("New custom column for regions");
}

/**
 * Returns a translated title for a region filter wizard.
 * 
 * @return	The translated title for a region filter wizard.
 */
QString RegionsTable::getNewFilterString() const
{
	return tr("New region filter");
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n region(s)".
 * 
 * @param numItems	The number of regions to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString RegionsTable::getItemCountString(int numItems) const
{
	return tr("%Ln region(s)", "", numItems);
}
