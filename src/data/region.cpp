#include "region.h"



Region::Region(int regionID, QString& name, int countryID) :
		regionID(regionID),
		name(name),
		countryID(countryID)
{}



bool Region::equalTo(Region* other)
{
	assert(other);
	if (name		!= other->name)			return false;
	if (countryID	!= other->countryID)	return false;
	return true;
}
