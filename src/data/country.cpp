#include "country.h"



Country::Country(ItemID countryID, QString& name) :
		countryID(countryID),
		name(name)
{}

Country::~Country()
{}



bool Country::equalTo(const Country* const other) const
{
	assert(other);
	if (name	!= other->name)	return false;
	return true;
}
