#include "country.h"



Country::Country(int countryID, QString& name) :
		countryID(countryID),
		name(name)
{}



bool Country::equalTo(Country* other)
{
	assert(other);
	if (name	!= other->name)	return false;
	return true;
}
