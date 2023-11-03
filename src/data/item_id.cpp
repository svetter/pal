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
 * @file ItemID.cpp
 * 
 * This file defines the ItemID class and its subclass ValidItemID.
 */

#include "item_id.h"

#include <QHashFunctions>
#include <assert.h>



const int ItemID::LOWEST_LEGAL_ID = 1;



/**
 * Creates an ItemID from an integer index.
 * 
 * @param id	The index.
 */
ItemID::ItemID(int id) : valid(id >= LOWEST_LEGAL_ID), id(id)
{}

/**
 * Creates an ItemID from a QVariant.
 * 
 * Returns an invalid ItemID if the QVariant cannot be converted to an integer.
 * 
 * @param id	The index as a QVariant.
 */
ItemID::ItemID(QVariant id) : valid(id.isValid() && id.canConvert<int>() && id.toInt() >= LOWEST_LEGAL_ID), id(id.toInt())
{}

/**
 * Creates an invalid ItemID.
 */
ItemID::ItemID() : valid(false), id(LOWEST_LEGAL_ID - 1)
{}



/**
 * Check whether this ItemID is valid.
 * 
 * @return	True if this ItemID is valid, false otherwise.
 */
bool ItemID::isValid() const
{
	return valid;
}

/**
 * Check whether this ItemID is invalid.
 * 
 * @return	True if this ItemID is invalid, false otherwise.
 */
bool ItemID::isInvalid() const
{
	return !valid;
}

/**
 * Get the integer index of this ItemID, assuming it is valid.
 * 
 * @pre This ItemID is valid. If it is not, an assertion fails.
 * 
 * @return	The integer index.
 */
int ItemID::get() const
{
	assert(valid);
	return id;
}

/**
 * Get the integer index of this ItemID.
 * 
 * If this ItemID is invalid, returns an invalid QVariant.
 * 
 * @return	A QVariant containing the integer index, or an invalid QVariant.
 */
QVariant ItemID::asQVariant() const
{
	if (isValid()) {
		return QVariant(id);
	} else {
		return QVariant();
	}
}

/**
 * Asserts that this ItemID is valid and returns an equivalent ValidItemID.
 * 
 * @pre This ItemID is valid. If it is not, an assertion fails.
 * 
 * @return	A ValidItemID with the same valid integer index.
 */
ValidItemID ItemID::forceValid() const
{
	assert(valid);
	return ValidItemID(id);
}





/**
 * Creates a ValidItemID from an integer index.
 * 
 * @pre The index represents a valid ID. If it does not, an assertion fails.
 * 
 * @param id	The index.
 */
ValidItemID::ValidItemID(int id) : ItemID(id)
{
	assert(isValid());
}

/**
 * Creates a ValidItemID from a QVariant.
 * 
 * @pre The QVariant represents a valid ID. If it does not, an assertion fails.
 * 
 * @param id	The index as a QVariant.
 */
ValidItemID::ValidItemID(QVariant id) : ItemID(id)
{
	assert(isValid());
}
