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



/**
 * Creates an ItemID from an integer index.
 * 
 * @param id	The index.
 */
ItemID::ItemID(int id, PALItemType type) :
	valid(id >= LOWEST_LEGAL_ID),
	id(id),
	type(type)
{}

/**
 * Creates an ItemID from a QVariant.
 * 
 * Returns an invalid ItemID if the QVariant cannot be converted to an integer.
 * 
 * @param id	The index as a QVariant.
 */
ItemID::ItemID(QVariant id, PALItemType type) :
	valid(id.isValid() && id.canConvert<int>() && id.toInt() >= LOWEST_LEGAL_ID),
	id(id.toInt()),
	type(type)
{}

/**
 * Creates an invalid ItemID.
 */
ItemID::ItemID(PALItemType type) :
	valid(false),
	id(LOWEST_LEGAL_ID - 1),
	type(type)
{}

/**
 * Creates an ItemID from another ItemID.
 *
 * @param id	A reference to the ItemID to copy.
 */
ItemID::ItemID(const ItemID& other) :
	valid(other.valid),
	id(other.id),
	type(other.type)
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

bool ItemID::isType(PALItemType itemType) const
{
	return itemType == type;
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
	return ValidItemID(id, type);
}


/**
 * Assigns another ItemID to this ItemID.
 * 
 * @param other	A reference to the ItemID to assign to this one.
 */
void ItemID::operator=(const ItemID& other)
{
	valid = other.valid;
	id = other.id;
}





/**
 * Creates a ValidItemID from an integer index.
 * 
 * @pre The index represents a valid ID. If it does not, an assertion fails.
 * 
 * @param id	The index.
 */
ValidItemID::ValidItemID(int id, PALItemType itemType) :
	ItemID(id, itemType)
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
ValidItemID::ValidItemID(QVariant id, PALItemType itemType) :
	ItemID(id, itemType)
{
	assert(isValid());
}

/**
 * Creates an ValidItemID from another ValidItemID.
 *
 * @param id	A reference to the ValidItemID to copy.
 */
ValidItemID::ValidItemID(const ValidItemID& other) :
	ValidItemID(other.asQVariant(), other.type)
{}


/**
 * Assigns another ValidItemID to this ValidItemID.
 *
 * @param other	A reference to the ValidItemID to assign to this one.
 */
void ValidItemID::operator=(const ValidItemID& other)
{
	ItemID::operator=(other);
	assert(isValid());
}





/**
 * Checks whether the given ItemIDs are equal to each other in validity and potentially value.
 * 
 * If both ItemIDs are invalid, they are considered equal.
 * Otherwise, they are considered equal if both are valid and have the same value.
 * 
 * @param id1	The first ItemID to compare.
 * @param id2	The second ItemID to compare.
 * @return		True if the ItemIDs are equal, false otherwise.
 */
bool operator==(const ItemID& id1, const ItemID& id2)
{
	if (id1.isInvalid() && id2.isInvalid()) return true;
	if (id1.isValid() != id2.isValid()) return false;
	return id1.get() == id2.get();
}

/**
 * Checks whether the given ItemIDs are not equal to each other in validity or potentially value.
 * 
 * If one ItemID is invalid and the other is valid, they are considered not equal.
 * Otherwise, they are considered not equal if both are valid and have different values.
 * 
 * @param id1	The first ItemID to compare.
 * @param id2	The second ItemID to compare.
 * @return		True if the ItemIDs are not equal, false otherwise.
 */
bool operator!=(const ItemID& id1, const ItemID& id2)
{
	return !operator==(id1, id2);
}



/**
 * Hashes the given ItemID using the given seed.
 * 
 * This implementation is required for using ItemID in QSets and implicitly comparing instances of
 * QList<ItemID>.
 * 
 * @param key	The ItemID to hash.
 * @param seed	The seed to use for hashing.
 * @return		The hash value.
 */
size_t qHash(const ItemID& key, size_t seed)
{
	return qHashMulti(seed, key.isValid(), key.isValid() ? key.get() : 0);
}
