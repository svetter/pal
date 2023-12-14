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
 * @file item_types.h
 * 
 * This file defines the PALItemType enum.
 */

#ifndef ITEM_TYPES_H
#define ITEM_TYPES_H



/**
 * Item type enumerator, encoding the type of an item (like ascent, peak...).
 */
enum PALItemType {
	ItemTypeAscent,
	ItemTypePeak,
	ItemTypeTrip,
	ItemTypeHiker,
	ItemTypeRegion,
	ItemTypeRange,
	ItemTypeCountry
};



#endif // ITEM_TYPES_H
