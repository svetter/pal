/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file photo.cpp
 * 
 * This file defines the internal representation of a photo item.
 */

#include "photo.h"



/**
 * Creates a new empty photo object with invalid values.
 */
Photo::Photo() :
	photoID(ItemID()),
	ascentID(ItemID()),
	sortIndex(-1),
	filepath(QString()),
	description(QString())
{}

/**
 * Creates a new empty photo object with the given properties.
 * 
 * @param photoID		The ID of the photo, if it already has one. Invalid ItemID otherwise.
 * @param ascentID		The ID of the ascent this photo belongs to, if specified. Invalid ItemID otherwise.
 * @param sortIndex		The sort index of this photo, if specified. -1 otherwise.
 * @param filepath		The path (including filename) of the photo, if specified. Empty QString otherwise.
 * @param description	The description of the photo, if specified. Empty QString otherwise.
 */
Photo::Photo(ItemID photoID, ItemID ascentID, int sortIndex, QString filepath, QString description) :
	photoID(photoID),
	ascentID(ascentID),
	sortIndex(sortIndex),
	filepath(filepath),
	description(description)
{}

/**
 * Creates a new photo object that is a copy of the given photo object.
 * 
 * @param originalPhoto	The photo object to copy.
 */
Photo::Photo(const Photo& originalPhoto) :
	QObject(),
	photoID(originalPhoto.photoID),
	ascentID(originalPhoto.ascentID),
	sortIndex(originalPhoto.sortIndex),
	filepath(originalPhoto.filepath),
	description(originalPhoto.description)
{}

/**
 * Destroys the photo object.
 */
Photo::~Photo()
{}



/**
 * Assigns the given photo object to this one.
 * 
 * @param other	The photo object to assign to this one.
 * @return		A reference to this photo object.
 */
Photo& Photo::operator=(const Photo& other)
{
	ascentID	= other.ascentID;
	filepath	= other.filepath;
	description	= other.description;
	return *this;
}

/**
 * Checks the given photo object for equality with this one.
 * 
 * @param other	The other photo object to compare this photo object with.
 * @return		True if the other photo object is equal to this one, false otherwise.
 */
bool Photo::operator==(const Photo& other) const
{
	if (ascentID	!= other.ascentID)		return false;
	if (filepath	!= other.filepath)		return false;
	if (description	!= other.description)	return false;
	return true;
}
