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

#include "peak.h"



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

Peak::~Peak()
{}



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



bool Peak::heightSpecified() const
{
	return height >= 0;
}



QVariant Peak::getHeightAsQVariant() const
{
	if (!heightSpecified()) return QVariant();
	return height;
}
