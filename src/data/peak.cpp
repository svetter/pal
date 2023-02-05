#include "peak.h"



Peak::Peak(int peakID, QString& name, int height, bool volcano, int regionID, QString& mapsLink, QString& earthLink, QString& wikiLink) :
		peakID(peakID),
		name(name),
		height(height),
		volcano(volcano),
		regionID(regionID),
		mapsLink(mapsLink),
		earthLink(earthLink),
		wikiLink(wikiLink)
{}



bool Peak::equalTo(Peak* other)
{
	if (name		!= other->name)			return false;
	if (height		!= other->height)		return false;
	if (volcano		!= other->volcano)		return false;
	if (regionID	!= other->regionID)		return false;
	if (mapsLink	!= other->mapsLink)		return false;
	if (earthLink	!= other->earthLink)	return false;
	if (wikiLink	!= other->wikiLink)		return false;
	return true;
}
