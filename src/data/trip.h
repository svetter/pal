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
 * @file trip.h
 * 
 * This file declares the internal representation of a trip item.
 */

#ifndef TRIP_H
#define TRIP_H

#include "item_id.h"

#include <QString>
#include <QDate>



/**
 * A class representing a trip item.
 */
class Trip : private QObject
{
	Q_OBJECT
	
public:
	/** The trip's internal ID. Invalid ItemID if this is a new item. */
	ItemID	tripID;
	/** The trip's name. Empty QString if not specified. */
	QString	name;
	/** The trip's start date. Invalid QDate if not specified. */
	QDate	startDate;
	/** The trip's end date. Invalid QDate if not specified. */
	QDate	endDate;
	/** The trip's description. Empty QString if not specified. */
	QString	description;
	
	Trip(ItemID tripID, QString& name, QDate& startDate, QDate& endDate, QString& description);
	virtual ~Trip();
	
	bool equalTo(const Trip& other) const;
	
	bool datesSpecified() const;
};



#endif // TRIP_H
