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
 * @file peak.cpp
 * 
 * This file defines the internal representation of a peak item.
 */

#include "peak.h"



/**
 * Creates a new peak object with the given properties.
 *
 * @param peakID	The ID of the peak, if it already has one. Invalid ItemID otherwise.
 * @param name		The name of the peak, if specified. Empty QString otherwise.
 * @param height	The height of the peak in meters, if specified. -1 otherwise.
 * @param volcano	Whether the peak is a volcano. Cannot be unspecified.
 * @param regionID	The ID of the region the peak is located in, if specified. Invalid ItemID otherwise.
 * @param mapsLink	A link to a Google Maps page showing the peak, if specified. Empty QString otherwise.
 * @param earthLink	A link to a Google Earth page showing the peak, if specified. Empty QString otherwise.
 * @param wikiLink	A link to a Wikipedia page about the peak, if specified. Empty QString otherwise.
 */
Peak::Peak(ItemID peakID, QString& name, int height, bool volcano, ItemID regionID, QString& mapsLink, QString& earthLink, QString& wikiLink) :
		peakID(peakID),
		name(name),
		height(height),
		volcano(volcano),
		regionID(regionID),
		mapsLink(mapsLink),
		earthLink(earthLink),
		wikiLink(wikiLink)
{}

/**
 * Destroys the peak object.
 */
Peak::~Peak()
{}



/**
 * Checks the given peak object for equality with this one.
 *
 * @param other	The other peak.
 * @return		True if the other peak is equal to this one, false otherwise.
 */
bool Peak::equalTo(const Peak* const other) const
{
	assert(other);
	if (name		!= other->name)			return false;
	if (height		!= other->height)		return false;
	if (volcano		!= other->volcano)		return false;
	if (regionID	!= other->regionID)		return false;
	if (mapsLink	!= other->mapsLink)		return false;
	if (earthLink	!= other->earthLink)	return false;
	if (wikiLink	!= other->wikiLink)		return false;
	return true;
}



/**
 * Checks whether the peak's height is specified.
 *
 * @return	True if the peak's height is specified, false otherwise.
 */
bool Peak::heightSpecified() const
{
	return height >= 0;
}



/**
 * Return the peak's height as a QVariant.
 * 
 * If the peak's height is not specified, an invalid QVariant is returned.
 *
 * @return	A QVariant containing the peak's height, or an invalid QVariant.
 */
QVariant Peak::getHeightAsQVariant() const
{
	if (!heightSpecified()) return QVariant();
	return height;
}
