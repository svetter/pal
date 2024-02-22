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
 * @file range.cpp
 * 
 * This file defines the internal representation of a mountain range item.
 */

#include "range.h"

#include <QCoreApplication>



/**
 * Creates a new mountain range object with the given properties.
 *
 * @param rangeID	The ID of the mountain range, if it already has one. Invalid ItemID otherwise.
 * @param name		The name of the mountain range, if specified. Empty QString otherwise.
 * @param continent	The continent the mountain range is located on, if specified. -1 otherwise.
 */
Range::Range(ItemID rangeID, QString& name, int continent) :
	rangeID(rangeID),
	name(name),
	continent(continent)
{}

/**
 * Destroys the mountain range object.
 */
Range::~Range()
{}



/**
 * Checks the given mountain range object for equality with this one.
 *
 * @param other	The other mountain range.
 * @return		True if the other mountain range is equal to this one, false otherwise.
 */
bool Range::equalTo(const Range& other) const
{
	if (name		!= other.name)		return false;
	if (continent	!= other.continent)	return false;
	return true;
}
