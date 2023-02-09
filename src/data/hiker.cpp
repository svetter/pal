#include "hiker.h"



Hiker::Hiker(int hikerID, QString& name) :
		hikerID(hikerID),
		name(name)
{}



bool Hiker::equalTo(Hiker* other)
{
	assert(other);
	if (name	!= other->name)	return false;
	return true;
}
