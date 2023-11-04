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
	inline static const int LOWEST_LEGAL_ID = 1;
	
public:
	ItemID(int id);
	ItemID(QVariant id);
	ItemID();
	ItemID(const ItemID& other);
	
	bool isValid() const;
	bool isInvalid() const;
	
	int get() const;
	QVariant asQVariant() const;
	
	ValidItemID forceValid() const;
	
	void operator=(const ItemID& other);
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
	ValidItemID(const ValidItemID& other);
	
	void operator=(const ItemID& other) = delete;
	void operator=(const ValidItemID& other);
};



bool operator==(const ItemID& id1, const ItemID& id2);
bool operator!=(const ItemID& id1, const ItemID& id2);

size_t qHash(const ItemID& key, size_t seed);



#endif // ITEM_ID_H
