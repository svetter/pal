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
 * @file trip.cpp
 * 
 * This file defines the internal representation of a trip item.
 */

#include "trip.h"



/**
 * Creates a new trip object with the given properties.
 * 
 * @param tripID		The ID of the trip item, if it already has one. Invalid ItemID otherwise.
 * @param name			The name of the trip, if specified. Empty QString otherwise.
 * @param startDate		The start date of the trip, if specified. Invalid QDate otherwise.
 * @param endDate		The end date of the trip, if specified. Invalid QDate otherwise.
 * @param description	The description of the trip, if specified. Empty QString otherwise.
 */
Trip::Trip(ItemID tripID, QString& name, QDate& startDate, QDate& endDate, QString& description) :
	tripID(tripID),
	name(name),
	startDate(startDate),
	endDate(endDate),
	description(description)
{}

/**
 * Destroys the trip object.
 */
Trip::~Trip()
{}



/**
 * Checks the given trip object for equality with this one.
 *
 * @param other	The other trip.
 * @return		True if the other trip is equal to this one, false otherwise.
 */
bool Trip::equalTo(const Trip& other) const
{
	if (name		!= other.name)			return false;
	if (startDate	!= other.startDate)		return false;
	if (endDate		!= other.endDate)		return false;
	if (description	!= other.description)	return false;
	return true;
}



/**
 * Checks whether the trip's start and end date are specified.
 *
 * @return	True if both the trip's dates are specified, false otherwise.
 */
bool Trip::datesSpecified() const
{
	return startDate.isValid() && endDate.isValid();
}
