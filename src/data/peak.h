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
 * @file peak.h
 * 
 * This file declares the internal representation of a peak item.
 */

#ifndef PEAK_H
#define PEAK_H

#include "item_id.h"

#include <QString>



/**
 * A class representing a peak item.
 */
class Peak : private QObject
{
	Q_OBJECT
	
public:
	/** The peak's internal ID. Invalid ItemID if this is a new item. */
	ItemID	peakID;
	/** The peak's name. Empty QString if not specified. */
	QString	name;
	/** The peak's height in meters. -1 if not specified. */
	int		height;
	/** Whether the peak is a volcano. Cannot be unspecified. */
	bool	volcano;
	/** The ID of the region the peak is in. Invalid ItemID if not specified. */
	ItemID	regionID;
	/** Link to the peak's location on Google Maps. Empty QString if not specified. */
	QString	mapsLink;
	/** Link to the peak's location on Google Earth. Empty QString if not specified. */
	QString	earthLink;
	/** Link to Wikipedia article about the peak. Empty QString if not specified. */
	QString	wikiLink;
	
	Peak(ItemID peakID, QString& name, int height, bool volcano, ItemID regionID, QString& mapsLink, QString& earthLink, QString& wikiLink);
	virtual ~Peak();
	
	bool equalTo(const Peak& other) const;
	
	bool heightSpecified() const;
	
	QVariant getHeightAsQVariant() const;
};



#endif // PEAK_H
