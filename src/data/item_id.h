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
 * @file ItemID.h
 * 
 * This file declares the ItemID class and its subclass ValidItemID.
 */

#ifndef ITEM_ID_H
#define ITEM_ID_H

#include <cstddef>
#include <QVariant>
#include <QHashFunctions>

class ValidItemID;



/**
 * A small wrapper class for an integer ID with a validity flag.
 * 
 * An invalid ItemID represents the absence of an ID in the database system.
 * It does **not** represent an ID for an item which no longer exists: The validity flag only
 * indicates whether the ID index is valid, not whether a reference using this ID is valid.
 */
class ItemID {
	/** Indicates whether the ID is valid. */
	bool valid;
	/** The ID index, if valid. */
	int id;
	
	/** The lowest integer that can be used as an ID in the database system. */
	static const int LOWEST_LEGAL_ID;
	
public:
	ItemID(int id);
	ItemID(QVariant id);
	ItemID();
	
	bool isValid() const;
	bool isInvalid() const;
	
	int get() const;
	QVariant asQVariant() const;
	
	ValidItemID forceValid() const;
};



/**
 * A small wrapper class for a valid integer ID.
 *
 * Using this class only indicates that the ID index is valid, not whether a reference using this
 * ID is valid.
 */
class ValidItemID : public ItemID {
public:
	ValidItemID(int id);
	ValidItemID(QVariant id);
};



/**
 * Checks whether the given ItemIDs are equal to each other in validity and potentially value.
 * 
 * If both ItemIDs are invalid, they are considered equal.
 Otherwise, they are considered equal if both are valid and have the same value.
 * 
 * @param id1	The first ItemID to compare.
 * @param id2	The second ItemID to compare.
 * @return		True if the ItemIDs are equal, false otherwise.
 */
inline bool operator==(const ItemID& id1, const ItemID& id2)
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
inline bool operator!=(const ItemID& id1, const ItemID& id2)
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
inline size_t qHash(const ItemID& key, size_t seed)
{
	return qHashMulti(seed, key.isValid(), key.isValid() ? key.get() : 0);
}



#endif // ITEM_ID_H
