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
 * @param db					The database to which the table belongs.
 * @param foreignRegionIDColumn	The primary key column of the RegionsTable.
 */
PeaksTable::PeaksTable(Database& db, PrimaryKeyColumn& foreignRegionIDColumn) :
	NormalTable(db, QString("Peaks"), tr("Peaks"), "peakID", tr("Peak ID")),
	//											name			uiName						type		nullable	foreignColumn
	nameColumn		(ValueColumn		(*this,	"name",			tr("Name"),					String,		false)),
	heightColumn	(ValueColumn		(*this,	"height",		tr("Height"),				Integer,	true)),
	volcanoColumn	(ValueColumn		(*this,	"volcano",		tr("Volcano"),				Bit,		false)),
	regionIDColumn	(ForeignKeyColumn	(*this,	"regionID",		tr("Region ID"),						true,		foreignRegionIDColumn)),
	mapsLinkColumn	(ValueColumn		(*this,	"mapsLink",		tr("Google Maps link"),		String,		true)),
	earthLinkColumn	(ValueColumn		(*this,	"earthLink",	tr("Google Earth link"),	String,		true)),
	wikiLinkColumn	(ValueColumn		(*this,	"wikiLink",		tr("Wikipedia link"),		String,		true))
{
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
BufferRowIndex PeaksTable::addRow(QWidget& parent, Peak& peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, peak);
	
	BufferRowIndex newPeakIndex = NormalTable::addRow(parent, columnDataPairs);
	peak.peakID = getPrimaryKeyAt(newPeakIndex);
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
void PeaksTable::updateRow(QWidget& parent, ValidItemID peakID, const Peak& peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, peak);
	
	NormalTable::updateRow(parent, peakID, columnDataPairs);
}

/**
 * Updates the contents of existing peaks in the table.
 * 
 * @param parent		The parent widget.
 * @param rowIndices	The indices of the peaks to update in the table buffer.
 * @param columns		The columns to update. Can not contain the primary key column.
 * @param peak			The new data for the peaks. The peakID will be ignored.
 */
void PeaksTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Peak& peak)
{
	assert(!columns.contains(&primaryKeyColumn));
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, peak);
	
	NormalTable::updateRows(parent, rowIndices, columnDataPairs);
}


/**
 * Translates the data of a peak to a list of column-data pairs.
 *
 * @param columns	The column list specifying the order of the data.
 * @param peak		The peak from which to get the data.
 * @return			A list of column-data pairs representing the peak's data.
 */
const QList<ColumnDataPair> PeaksTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Peak& peak) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &nameColumn)			{ data = peak.name;						}
		else if (column == &heightColumn)		{ data = peak.getHeightAsQVariant();	}
		else if (column == &volcanoColumn)		{ data = peak.volcano;					}
		else if (column == &regionIDColumn)		{ data = peak.regionID.asQVariant();	}
		else if (column == &mapsLinkColumn)		{ data = peak.mapsLink;					}
		else if (column == &earthLinkColumn)	{ data = peak.earthLink;				}
		else if (column == &wikiLinkColumn)		{ data = peak.wikiLink;					}
		else assert(false);
		
		if (column->type == String && data.toString().isEmpty()) data = QVariant();
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns the translation of "Peak" (singular).
 *
 * @return	The translation of "Peak" (singular).
 */
QString PeaksTable::getItemNameSingular() const
{
	return tr("Peak");
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
 * Returns a translated message confirming that a new peak has been created.
 * 
 * @return	The translated message confirming creation of a new peak.
 */
QString PeaksTable::getCreationConfirmMessage() const
{
	return tr("Saved new peak.");
}

/**
 * Returns a translated message confirming that a number of peaks have been edited.
 *
 * @param numEdited	The number of peaks that have been edited.
 * @return			The translated message confirming the editing of the peaks.
 */
QString PeaksTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln peak(s).", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of peaks have been deleted.
 *
 * @param numDeleted	The number of peaks that have been deleted.
 * @return				The translated message confirming the deletion of the peaks.
 */
QString PeaksTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln peak(s).", "", numDeleted);
}

/**
 * Returns a translated title for a peak filter wizard.
 * 
 * @return	The translated title for a peak filter wizard.
 */
QString PeaksTable::getNewFilterString() const
{
	return tr("New peak filter");
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n peak(s)".
 * 
 * @param numItems	The number of peaks to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString PeaksTable::getItemCountString(int numItems) const
{
	return tr("%Ln peak(s)", "", numItems);
}
