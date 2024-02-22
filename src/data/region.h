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
 * @file region.h
 * 
 * This file declares the internal representation of a region item.
 */

#ifndef REGION_H
#define REGION_H

#include "item_id.h"

#include <QString>



/**
 * A class representing a region item.
 */
class Region : private QObject
{
	Q_OBJECT
	
public:
	/** The region's internal ID. Invalid ItemID if this is a new item. */
	ItemID	regionID;
	/** The regions's name. Empty QString if not specified. */
	QString	name;
	/** The ID of the range this region belongs to. Invalid ItemID if not specified. */
	ItemID	rangeID;
	/** The ID of the country this region belongs to. Invalid ItemID if not specified. */
	ItemID	countryID;
	
	Region(ItemID regionID, QString& name, ItemID rangeID, ItemID countryID);
	virtual ~Region();
	
	bool equalTo(const Region& other) const;
};



#endif // REGION_H
