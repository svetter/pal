#include "trip.h"



Trip::Trip(int tripID, QString& name, QDate& startDate, QDate& endDate, QString& description) :
		tripID(tripID),
		name(name),
		startDate(startDate),
		endDate(endDate),
		description(description)
{}



bool Trip::equalTo(Trip* other)
{
	if (name		!= other->name)			return false;
	if (startDate	!= other->startDate)	return false;
	if (endDate		!= other->endDate)		return false;
	if (description	!= other->description)	return false;
	return true;
}
