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
 * @file country.h
 * 
 * This file declares the internal representation of a country item.
 */

#ifndef COUNTRY_H
#define COUNTRY_H

#include "item_id.h"

#include <QString>



/**
 * A class representing a country item.
 */
class Country : private QObject
{
	Q_OBJECT
	
public:
	/** The country's internal ID. Invalid ItemID if this is a new item. */
	ItemID	countryID;
	/** The country's name. Empty QString if not specified. */
	QString	name;
	
	Country(ItemID countryID, QString& name);
	virtual ~Country();
	
	bool equalTo(const Country& other) const;
};



#endif // COUNTRY_H
