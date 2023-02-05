#include "range.h"



Range::Range(int rangeID, QString& name, int continent) :
		rangeID(rangeID),
		name(name),
		continent(continent)
{}



bool Range::equalTo(Range* other)
{
	if (name		!= other->name)			return false;
	if (continent	!= other->continent)	return false;
	return true;
}
