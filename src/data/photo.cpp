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

#include "photo.h"



Photo::Photo() :
		photoID(ItemID()),
		ascentID(ItemID()),
		sortIndex(-1),
		filepath(QString()),
		description(QString())
{}

Photo::Photo(ItemID photoID, ItemID ascentID, int sortIndex, QString filepath, QString description) :
		photoID(photoID),
		ascentID(ascentID),
		sortIndex(sortIndex),
		filepath(filepath),
		description(description)
{}

Photo::Photo(const Photo& originalPhoto) :
		QObject(),
		photoID(originalPhoto.photoID),
		ascentID(originalPhoto.ascentID),
		sortIndex(originalPhoto.sortIndex),
		filepath(originalPhoto.filepath),
		description(originalPhoto.description)
{}

Photo::~Photo()
{}



Photo& Photo::operator=(const Photo& other)
{
	ascentID	= other.ascentID;
	filepath	= other.filepath;
	description	= other.description;
	return *this;
}

bool Photo::operator==(const Photo& other) const
{
	if (ascentID	!= other.ascentID)		return false;
	if (filepath	!= other.filepath)		return false;
	if (description	!= other.description)	return false;
	return true;
}
