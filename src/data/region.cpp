#include "region.h"



Region::Region(ItemID regionID, QString& name, ItemID rangeID, ItemID countryID) :
		regionID(regionID),
		name(name),
		rangeID(rangeID),
		countryID(countryID)
{}

Region::~Region()
{}



bool Region::equalTo(const Region* const other) const
{
	assert(other);
	if (name		!= other->name)			return false;
	if (rangeID		!= other->rangeID)		return false;
	if (countryID	!= other->countryID)	return false;
	return true;
}
