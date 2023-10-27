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
 * @file country.cpp
 * 
 * This file defines the internal representation of an country item.
 */

#include "country.h"



/**
 * Creates a new country object with the given properties.
 * 
 * @param countryID	The ID of the country item, if it already has one. Invalid ItemID otherwise.
 * @param name		The name of the country, if specified. Empty QString otherwise.
 */
Country::Country(ItemID countryID, QString& name) :
		countryID(countryID),
		name(name)
{}

/**
 * Destroys the country object.
 */
Country::~Country()
{}



/**
 * Checks the given country object for equality with this one.
 *
 * @param other	The other country.
 * @return		True if the other country is equal to this one, false otherwise.
 */
bool Country::equalTo(const Country* const other) const
{
	assert(other);
	if (name	!= other->name)	return false;
	return true;
}
