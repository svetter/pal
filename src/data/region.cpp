#include "region.h"



Region::Region(int regionID, QString& name, int rangeID, int countryID) :
		regionID(regionID),
		name(name),
		rangeID(rangeID),
		countryID(countryID)
{}



bool Region::equalTo(Region* other)
{
	assert(other);
	if (name		!= other->name)			return false;
	if (rangeID		!= other->rangeID)		return false;
	if (countryID	!= other->countryID)	return false;
	return true;
}
