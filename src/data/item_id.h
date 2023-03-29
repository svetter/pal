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

#ifndef ITEM_ID_H
#define ITEM_ID_H

#include <cstddef>
#include <QVariant>
#include <QHashFunctions>

class ValidItemID;



class ItemID {
	bool valid;
	int id;
	
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



class ValidItemID : public ItemID {
public:
	ValidItemID(int id);
	ValidItemID(QVariant id);
};



inline bool operator==(const ItemID& id1, const ItemID& id2)
{
	if (id1.isInvalid() && id2.isInvalid()) return true;
	if (id1.isValid() != id2.isValid()) return false;
	return id1.get() == id2.get();
}

inline bool operator!=(const ItemID& id1, const ItemID& id2)
{
	return !operator==(id1, id2);
}

inline size_t qHash(const ItemID& key, size_t seed)
{
	return qHashMulti(seed, key.isValid(), key.isValid() ? key.get() : 0);
}



#endif // ITEM_ID_H
