#include "range.h"

#include <QCoreApplication>



Range::Range(int rangeID, QString& name, int continent) :
		rangeID(rangeID),
		name(name),
		continent(continent)
{}



bool Range::equalTo(Range* other)
{
	assert(other);
	if (name		!= other->name)			return false;
	if (continent	!= other->continent)	return false;
	return true;
}



QString Range::tr(const char* string)
{
	return QCoreApplication::translate("Range", string);
}
