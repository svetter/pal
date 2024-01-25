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
 * @file photo.h
 * 
 * This file declares the internal representation of a photo item.
 */

#ifndef PHOTO_H
#define PHOTO_H

#include "item_id.h"

#include <QString>



/**
 * A class representing a photo item.
 */
class Photo : private QObject
{
	Q_OBJECT
	
public:
	/** The photo's internal ID. Invalid ItemID if this is a new item. */
	ItemID	photoID;
	/** The ID of the ascent this photo belongs to. Invalid ItemID if the ascent is a new item. */
	ItemID	ascentID;
	/** The sort index of this photo in the ascent's photo list. -1 if not specified. */
	int		sortIndex;
	/** The path (including filename) to the image file, if specified. Empty QString otherwise. */
	QString	filepath;
	/** The description of the photo, if specified. Empty QString otherwise. */
	QString	description;
	
	Photo();
	Photo(ItemID photoID, ItemID ascentID, int sortIndex, QString filepath, QString description);
	Photo(const Photo& originalPhoto);
	virtual ~Photo();
	
	Photo& operator=(const Photo& other);
	
	bool operator==(const Photo& other) const;
};



#endif // PHOTO_H
