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

#include "trip.h"



Trip::Trip(ItemID tripID, QString& name, QDate& startDate, QDate& endDate, QString& description) :
		tripID(tripID),
		name(name),
		startDate(startDate),
		endDate(endDate),
		description(description)
{}

Trip::~Trip()
{}



bool Trip::equalTo(const Trip* const other) const
{
	assert(other);
	if (name		!= other->name)			return false;
	if (startDate	!= other->startDate)	return false;
	if (endDate		!= other->endDate)		return false;
	if (description	!= other->description)	return false;
	return true;
}



bool Trip::datesSpecified() const
{
	return startDate.isValid() && endDate.isValid();
}
