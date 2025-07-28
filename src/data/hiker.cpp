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
 * @file hiker.cpp
 * 
 * This file defines the internal representation of a hiker item.
 */

#include "hiker.h"



/**
 * Creates a new hiker object with the given properties.
 *
 * @param hikerID	The ID of the hiker item, if it already has one. Invalid ItemID otherwise.
 * @param name		The name of the hiker, if specified. Empty QString otherwise.
 */
Hiker::Hiker(ItemID hikerID, QString& name) :
	hikerID(hikerID),
	name(name)
{}

/**
 * Destroys the hiker object.
 */
Hiker::~Hiker()
{}



/**
 * Checks the given hiker object for equality with this one.
 *
 * @param other	The other hiker.
 * @return		True if the other hiker is equal to this one, false otherwise.
 */
bool Hiker::equalTo(const Hiker& other) const
{
	if (name	!= other.name)	return false;
	return true;
}
