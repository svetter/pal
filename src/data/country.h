#ifndef COUNTRY_H
#define COUNTRY_H

#include "item_id.h"

#include <QString>



class Country : private QObject
{
	Q_OBJECT
	
public:
	ItemID	countryID;
	QString	name;
	
	Country(ItemID countryID, QString& name);
	virtual ~Country();
	
	bool equalTo(const Country* const other) const;
};



#endif // COUNTRY_H
