#ifndef COUNTRY_H
#define COUNTRY_H

#include "item_id.h"

#include <QString>



class Country
{
public:
	ItemID	countryID;
	QString	name;
	
	Country(ItemID countryID, QString& name);
	
	bool equalTo(const Country* const other) const;
};



#endif // COUNTRY_H
