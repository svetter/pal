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
 * @file trips_table.h
 * 
 * This file defines the TripsTable class.
 */

#include "trips_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new TripsTable.
 * 
 * @param db	The database to which the table belongs.
 */
TripsTable::TripsTable(Database& db) :
	NormalTable(db, QString("Trips"), tr("Trips"), "tripID", tr("Trip ID")),
	//										name			uiName				type	nullable
	nameColumn			(ValueColumn(*this,	"name",			tr("Name"),			String,	false)),
	startDateColumn		(ValueColumn(*this,	"startDate",	tr("Start date"),	Date,	true)),
	endDateColumn		(ValueColumn(*this,	"endDate",		tr("End date"),		Date,	true)),
	descriptionColumn	(ValueColumn(*this,	"description",	tr("Description"),	String,	true))
{
	addColumn(nameColumn);
	addColumn(startDateColumn);
	addColumn(endDateColumn);
	addColumn(descriptionColumn);
}



/**
 * Adds a new trip to the table.
 *
 * @param parent	The parent widget.
 * @param trip		The trip to add.
 * @return			The index of the new trip in the table buffer.
 */
BufferRowIndex TripsTable::addRow(QWidget& parent, Trip& trip)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, trip);
	
	BufferRowIndex newTripIndex = NormalTable::addRow(parent, columnDataPairs);
	trip.tripID = getPrimaryKeyAt(newTripIndex);
	return newTripIndex;
}

/**
 * Updates the contents of an existing trip in the table.
 *
 * @pre The given trip's ItemID will be used to identify the trip to update and thus must be valid.
 *
 * @param parent	The parent widget.
 * @param trip		The trip to update.
 */
void TripsTable::updateRow(QWidget& parent, ValidItemID tripID, const Trip& trip)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, trip);
	
	NormalTable::updateRow(parent, tripID, columnDataPairs);
}

/**
 * Updates the contents of existing trips in the table.
 * 
 * @param parent		The parent widget.
 * @param rowIndices	The indices of the trips to update in the table buffer.
 * @param columns		The columns to update. Can not contain the primary key column.
 * @param trip			The new data for the trips. The tripID will be ignored.
 */
void TripsTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Trip& trip)
{
	assert(!columns.contains(&primaryKeyColumn));
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, trip);
	
	NormalTable::updateRows(parent, rowIndices, columnDataPairs);
}


/**
 * Translates the data of a trip to a list of column-data pairs.
 *
 * @param columns	The column list specifying the order of the data.
 * @param trip		The trip from which to get the data.
 * @return			A list of column-data pairs representing the trip's data.
 */
const QList<ColumnDataPair> TripsTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Trip& trip) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &nameColumn)			{ data = trip.name;			}
		else if (column == &startDateColumn)	{ data = trip.startDate;	}
		else if (column == &endDateColumn)		{ data = trip.endDate;		}
		else if (column == &descriptionColumn)	{ data = trip.description;	}
		else assert(false);
		
		if (column->type == String && data.toString().isEmpty()) data = QVariant();
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns the translated string to be displayed to indicate that no trip is selected.
 *
 * @return	The translated string representing absence of a trip.
 */
QString TripsTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns a translated message confirming that a new trip has been created.
 * 
 * @return	The translated message confirming creation of a new trip.
 */
QString TripsTable::getCreationConfirmMessage() const
{
	return tr("Saved new trip.");
}

/**
 * Returns a translated message confirming that a number of trips have been edited.
 *
 * @param numEdited	The number of trips that have been edited.
 * @return			The translated message confirming the editing of the trips.
 */
QString TripsTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln trip(s).", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of trips have been deleted.
 *
 * @param numDeleted	The number of items that have been deleted.
 * @return				The translated message confirming the deletion of the trips.
 */
QString TripsTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln trip(s).", "", numDeleted);
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n trip(s)".
 * 
 * @param numItems	The number of trips to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString TripsTable::getItemCountString(int numItems) const
{
	return tr("%Ln trip(s)", "", numItems);
}
