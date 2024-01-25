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
 * @file peaks_table.h
 * 
 * This file defines the PeaksTable class.
 */

#include "peaks_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new PeaksTable.
 * 
 * @param foreignRegionIDColumn	The primary key column of the RegionsTable.
 */
PeaksTable::PeaksTable(PrimaryKeyColumn* foreignRegionIDColumn) :
	NormalTable(QString("Peaks"), tr("Peaks"), "peakID", tr("Peak ID")),
	//										name					uiName						type		nullable	foreignColumn
	nameColumn		(new ValueColumn		(this,	"name",			tr("Name"),					String,		false)),
	heightColumn	(new ValueColumn		(this,	"height",		tr("Height"),				Integer,	true)),
	volcanoColumn	(new ValueColumn		(this,	"volcano",		tr("Volcano"),				Bit,		false)),
	regionIDColumn	(new ForeignKeyColumn	(this,	"regionID",		tr("Region ID"),						true,		foreignRegionIDColumn)),
	mapsLinkColumn	(new ValueColumn		(this,	"mapsLink",		tr("Google Maps link"),		String,		true)),
	earthLinkColumn	(new ValueColumn		(this,	"earthLink",	tr("Google Earth link"),	String,		true)),
	wikiLinkColumn	(new ValueColumn		(this,	"wikiLink",		tr("Wikipedia link"),		String,		true))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
	addColumn(heightColumn);
	addColumn(volcanoColumn);
	addColumn(regionIDColumn);
	addColumn(mapsLinkColumn);
	addColumn(earthLinkColumn);
	addColumn(wikiLinkColumn);
}



/**
 * Adds a new peak to the table.
 *
 * @param parent	The parent widget.
 * @param peak		The peak to add.
 * @return			The index of the new peak in the table buffer.
 */
BufferRowIndex PeaksTable::addRow(QWidget* parent, Peak* peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, peak);
	
	BufferRowIndex newPeakIndex = NormalTable::addRow(parent, columnDataPairs);
	peak->peakID = getPrimaryKeyAt(newPeakIndex);
	return newPeakIndex;
}

/**
 * Updates the contents of an existing peak in the table.
 *
 * @pre The given peak's ItemID will be used to identify the peak to update and thus must be valid.
 *
 * @param parent	The parent widget.
 * @param peak		The peak to update.
 */
void PeaksTable::updateRow(QWidget* parent, ValidItemID peakID, const Peak* peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, peak);
	
	NormalTable::updateRow(parent, peakID, columnDataPairs);
}


/**
 * Translates the data of a peak to a list of column-data pairs.
 *
 * @param columns	The column list specifying the order of the data.
 * @param peak		The peak from which to get the data.
 * @return			A list of column-data pairs representing the peak's data.
 */
const QList<ColumnDataPair> PeaksTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Peak* peak) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == nameColumn)		{ data = peak->name;					}
		else if (column == heightColumn)	{ data = peak->getHeightAsQVariant();	}
		else if (column == volcanoColumn)	{ data = peak->volcano;					}
		else if (column == regionIDColumn)	{ data = peak->regionID.asQVariant();	}
		else if (column == mapsLinkColumn)	{ data = peak->mapsLink;				}
		else if (column == earthLinkColumn)	{ data = peak->earthLink;				}
		else if (column == wikiLinkColumn)	{ data = peak->wikiLink;				}
		else assert(false);
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns the translated string to be displayed to indicate that no peak is selected.
 *
 * @return	The translated string representing absence of a peak.
 */
QString PeaksTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns the translation of "peak" (singular), not capitalized unless the language requires it.
 *
 * @return	The translation of "peak" (singular) for use mid-sentence.
 */
QString PeaksTable::getItemNameSingularLowercase() const
{
	return tr("peak");
}

/**
 * Returns the translation of "peaks" (plural), not capitalized unless the language requires it.
 *
 * @return	The translation of "peaks" (plural) for use mid-sentence.
 */
QString PeaksTable::getItemNamePluralLowercase() const
{
	return tr("peaks");
}
