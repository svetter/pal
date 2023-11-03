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
 * This file declares the TripsTable class.
 */

#ifndef TRIPS_TABLE_H
#define TRIPS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/trip.h"

#include <QWidget>



/**
 * A class for accessing and manipulating the ascents table in the database.
 */
class TripsTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The name column. */
	Column* const nameColumn;
	/** The start date column. */
	Column* const startDateColumn;
	/** The end date column. */
	Column* const endDateColumn;
	/** The description column. */
	Column* const descriptionColumn;
	
	TripsTable();
	
	BufferRowIndex addRow(QWidget* parent, Trip* trip);
	void updateRow(QWidget* parent, ValidItemID tripID, const Trip* trip);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Trip* trip) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // TRIPS_TABLE_H
