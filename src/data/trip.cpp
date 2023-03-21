#include "trip.h"



Trip::Trip(ItemID tripID, QString& name, QDate& startDate, QDate& endDate, QString& description) :
		tripID(tripID),
		name(name),
		startDate(startDate),
		endDate(endDate),
		description(description)
{}

Trip::~Trip()
{}



bool Trip::equalTo(const Trip* const other) const
{
	assert(other);
	if (name		!= other->name)			return false;
	if (startDate	!= other->startDate)	return false;
	if (endDate		!= other->endDate)		return false;
	if (description	!= other->description)	return false;
	return true;
}



bool Trip::datesSpecified() const
{
	return startDate.isValid() && endDate.isValid();
}
