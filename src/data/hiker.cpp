#include "hiker.h"



Hiker::Hiker(ItemID hikerID, QString& name) :
		hikerID(hikerID),
		name(name)
{}



bool Hiker::equalTo(const Hiker* const other) const
{
	assert(other);
	if (name	!= other->name)	return false;
	return true;
}
