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
 * @file region.cpp
 * 
 * This file defines the internal representation of a region item.
 */

#include "region.h"



/**
 * Creates a new region object with the given properties.
 *
 * @param regionID		The ID of the region, if it already has one. Invalid ItemID otherwise.
 * @param name			The name of the region, if specified. Empty QString otherwise.
 * @param rangeID		The ID of the mountain range the region belongs to, if specified. Invalid ItemID otherwise.
 * @param countryID	The ID of the country the region belongs to, if specified. Invalid ItemID otherwise.
 */
Region::Region(ItemID regionID, QString& name, ItemID rangeID, ItemID countryID) :
		regionID(regionID),
		name(name),
		rangeID(rangeID),
		countryID(countryID)
{}

/**
 * Destroys the region object.
 */
Region::~Region()
{}



/**
 * Checks the given region object for equality with this one.
 *
 * @param other	The other region.
 * @return		True if the other region is equal to this one, false otherwise.
 */
bool Region::operator==(const Region* const other) const
{
	assert(other);
	if (name		!= other->name)			return false;
	if (rangeID		!= other->rangeID)		return false;
	if (countryID	!= other->countryID)	return false;
	return true;
}
