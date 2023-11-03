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
		NormalTable(QString("Trips"), tr("Trips"), "tripID"),
		//								name			uiName				type	nullable	primaryKey	foreignKey	inTable
		nameColumn			(new Column("name",			tr("Name"),			String,	false,		false,		nullptr,	this)),
		startDateColumn		(new Column("startDate",	tr("Start date"),	Date,	true,		false,		nullptr,	this)),
		endDateColumn		(new Column("endDate",		tr("End date"),		Date,	true,		false,		nullptr,	this)),
		descriptionColumn	(new Column("description",	tr("Description"),	String,	true,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
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
BufferRowIndex TripsTable::addRow(QWidget* parent, Trip* trip)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, trip);
	
	BufferRowIndex newTripIndex = NormalTable::addRow(parent, columns, data);
	trip->tripID = getPrimaryKeyAt(newTripIndex);
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
void TripsTable::updateRow(QWidget* parent, ValidItemID tripID, const Trip* trip)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, trip);
	
	NormalTable::updateRow(parent, tripID, columns, data);
}


/**
 * Translates the data of a trip to a list of QVariants.
 *
 * @param columns	The column list specifying the order of the data.
 * @param trip		The trip from which to get the data.
 * @return			The list of QVariants representing the trip's data.
 */
QList<QVariant> TripsTable::mapDataToQVariantList(QList<const Column*>& columns, const Trip* trip) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)			{ data.append(trip->name);			continue; }
		if (column == startDateColumn)		{ data.append(trip->startDate);		continue; }
		if (column == endDateColumn)		{ data.append(trip->endDate);		continue; }
		if (column == descriptionColumn)	{ data.append(trip->description);	continue; }
		assert(false);
	}
	return data;
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
