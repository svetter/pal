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
