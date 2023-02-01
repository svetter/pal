#ifndef TRIP_H
#define TRIP_H

#include <QString>
#include <QDate>



class Trip
{
public:
	int			tripID;
	QString		name;
	QDate*		startDate;
	QDate*		endDate;
	QString*	notes;
};



#endif // TRIP_H
