#ifndef TRIP_H
#define TRIP_H

#include <QString>
#include <QDate>



class Trip
{
public:
	int		tripID;
	QString	name;
	QDate	startDate;
	QDate	endDate;
	QString	notes;
	
	bool equalTo(Ascent* other) {
		if (name != other->name)			return false;
		if (startDate != other->startDate)	return false;
		if (endDate != other->endDate)		return false;
		if (notes != other->notes)			return false;
		return true;
	}
};



#endif // TRIP_H
