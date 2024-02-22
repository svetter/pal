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
 * @file hiker.h
 * 
 * This file declares the internal representation of a hiker item.
 */

#ifndef HIKER_H
#define HIKER_H

#include "item_id.h"

#include <QString>



/**
 * A class representing a hiker item.
 */
class Hiker : private QObject
{
	Q_OBJECT
	
public:
	/** The hiker's internal ID. Invalid ItemID if this is a new item. */
	ItemID	hikerID;
	/** The hiker's name. Empty QString if not specified. */
	QString	name;
	
	Hiker(ItemID hikerID, QString& name);
	virtual ~Hiker();
	
	bool equalTo(const Hiker& other) const;
};



#endif // HIKER_H
