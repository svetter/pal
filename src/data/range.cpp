#include "range.h"

#include <QCoreApplication>



Range::Range(ItemID rangeID, QString& name, int continent) :
		rangeID(rangeID),
		name(name),
		continent(continent)
{}

Range::~Range()
{}



bool Range::equalTo(const Range* const other) const
{
	assert(other);
	if (name		!= other->name)			return false;
	if (continent	!= other->continent)	return false;
	return true;
}
