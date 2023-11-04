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
 * @file range.h
 * 
 * This file declares the internal representation of a mountain range item.
 */

#ifndef RANGE_H
#define RANGE_H

#include "item_id.h"

#include <QStringList>
#include <QTranslator>



/**
 * A class representing a mountain range item.
 */
class Range : private QObject
{
	Q_OBJECT
	
public:
	/** The mountain range's internal ID. Invalid ItemID if this is a new item. */
	ItemID	rangeID;
	/** The mountain range's name. Empty QString if not specified. */
	QString	name;
	/** The continent this mountain range is located on. -1 if not specified. */
	int		continent;
	
	Range(ItemID rangeID, QString& name, int continent);
	virtual ~Range();
	
	bool operator==(const Range* const other) const;
};



#endif // RANGE_H
