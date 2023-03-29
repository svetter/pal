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

#ifndef REGION_H
#define REGION_H

#include "item_id.h"

#include <QString>



class Region : private QObject
{
	Q_OBJECT
	
public:
	ItemID	regionID;
	QString	name;
	ItemID	rangeID;
	ItemID	countryID;
	
	Region(ItemID regionID, QString& name, ItemID rangeID, ItemID countryID);
	virtual ~Region();
	
	bool equalTo(const Region* const other) const;
};



#endif // REGION_H
