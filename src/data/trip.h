#ifndef TRIP_H
#define TRIP_H

#include "item_id.h"

#include <QString>
#include <QDate>



class Trip : private QObject
{
	Q_OBJECT
	
public:
	ItemID	tripID;
	QString	name;
	QDate	startDate;
	QDate	endDate;
	QString	description;
	
	Trip(ItemID tripID, QString& name, QDate& startDate, QDate& endDate, QString& description);
	virtual ~Trip();
	
	bool equalTo(const Trip* const other) const;
	
	bool datesSpecified() const;
};



#endif // TRIP_H
