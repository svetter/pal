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
	QString	description;
	
	Trip(int tripID, QString& name, QDate& startDate, QDate& endDate, QString& description);
	
	bool equalTo(Trip* other);
};



#endif // TRIP_H
