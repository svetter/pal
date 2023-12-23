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
 * This file declares the ItemID class and its subclass ValidItemID, and defines
 * the ItemIDPrivilegedFunctionAccessor class and corresponding macros.
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
	inline static const int LOWEST_LEGAL_ID = 1;
	
public:
	ItemID(int id);
	ItemID(QVariant id);
	ItemID();
	ItemID(const ItemID& other);
	
	bool isValid() const;
	bool isInvalid() const;
	
private:
	int get() const;
public:
	QVariant asQVariant() const;
	
private:
	ValidItemID forceValid() const;
	
public:
	void operator=(const ItemID& other);
	
	friend class ItemIDPrivilegedFunctionAccessor;
	friend bool operator==(const ItemID& id1, const ItemID& id2);
	friend size_t qHash(const ItemID& key, size_t seed);
};



/**
 * A small wrapper class for a valid integer ID.
 *
 * Using this class only indicates that the ID index is valid, not whether a reference using this ID
 * is valid.
 */
class ValidItemID : public ItemID {
private:
	ValidItemID(int id);
	ValidItemID(QVariant id);
public:
	ValidItemID(const ValidItemID& other);
	
	void operator=(const ItemID& other) = delete;
	void operator=(const ValidItemID& other);
	
	friend class ItemID;
};



bool operator==(const ItemID& id1, const ItemID& id2);
bool operator!=(const ItemID& id1, const ItemID& id2);

size_t qHash(const ItemID& key, size_t seed);



/**
 * A macro to get the integer index of an ItemID.
 * 
 * Performs an assertion at caller level that the ItemID must be valid.
 * Always use this macro to make sure that assertions are performed at caller level first, to make
 * sure that violated assertions give a useful error message.
 * 
 * @param item_id	The ItemID to get the integer index of.
 * @return			The integer index of the ItemID.
*/
#define ID_GET(item_id) (assert(item_id.isValid()), ItemIDPrivilegedFunctionAccessor::getValueForItemID(item_id))

/**
 * A macro to force an ItemID to be valid, turning it into a ValidItemID.
 * 
 * Performs an assertion at caller level that the ItemID must be valid.
 * Always use this macro to make sure that assertions are performed at caller level first, to make
 * sure that violated assertions give a useful error message.
 * 
 * @param item_id	The ItemID to force to be valid.
 * @return			The ValidItemID created from the ItemID.
 */
#define FORCE_VALID(item_id) (assert(item_id.isValid()), ItemIDPrivilegedFunctionAccessor::forceItemIDValid(item_id))

/**
 * A macro to create a ValidItemID from an int.
 * 
 * Performs an assertion at caller level that the integer must produce a valid ItemID.
 * Always use this macro to make sure that assertions are performed at caller level first, to make
 * sure that violated assertions give a useful error message.
 * 
 * @param integer	The integer to create a ValidItemID from.
 * @return			The ValidItemID created from the integer.
 */
#define VALID_ITEM_ID(integer) (assert(ItemID(integer).isValid()), ItemIDPrivilegedFunctionAccessor::forceItemIDValid(ItemID(integer)))


/**
 * A class which serves as an indirection tool to make functions of ItemID which require assertions
 * publicly accessible, but in a way that makes it clear that they should not be used directly.
 * 
 * Instead, the macros ID_GET, FORCE_VALID and VALID_ITEM_ID, which this is designed to work with,
 * should be used.
 */
class ItemIDPrivilegedFunctionAccessor
{
public:
	/** Do NOT call directly! Use macro ID_GET(...) instead. */
	inline static int			getValueForItemID	(ItemID itemID)	{ return itemID.get();			}
	/** Do NOT call directly! Use macros FORCE_VALID(...) and VALID_ITEM_ID(...) instead. */
	inline static ValidItemID	forceItemIDValid	(ItemID itemID)	{ return itemID.forceValid();	}
};



#endif // ITEM_ID_H
