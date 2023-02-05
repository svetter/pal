#include "trip.h"



Trip::Trip(int tripID, QString& name, QDate& startDate, QDate& endDate, QString& notes) :
		tripID(tripID),
		name(name),
		startDate(startDate),
		endDate(endDate),
		notes(notes)
{}



bool Trip::equalTo(Trip* other)
{
	if (name		!= other->name)			return false;
	if (startDate	!= other->startDate)	return false;
	if (endDate		!= other->endDate)		return false;
	if (notes		!= other->notes)		return false;
	return true;
}
