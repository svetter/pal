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
 */
TripsTable::TripsTable() :
	NormalTable(QString("Trips"), tr("Trips"), "tripID", tr("Trip ID")),
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
BufferRowIndex TripsTable::addRow(QWidget* parent, Trip& trip)
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
void TripsTable::updateRow(QWidget* parent, ValidItemID tripID, const Trip& trip)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, trip);
	
	NormalTable::updateRow(parent, tripID, columnDataPairs);
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
 * Returns the translation of "trip" (singular), not capitalized unless the language requires it.
 *
 * @return	The translation of "trip" (singular) for use mid-sentence.
 */
QString TripsTable::getItemNameSingularLowercase() const
{
	return tr("trip");
}

/**
 * Returns the translation of "trips" (plural), not capitalized unless the language requires it.
 *
 * @return	The translation of "trips" (plural) for use mid-sentence.
 */
QString TripsTable::getItemNamePluralLowercase() const
{
	return tr("trips");
}
