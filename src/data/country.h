#ifndef COUNTRY_H
#define COUNTRY_H

#include <QString>



class Country
{
public:
	int		countryID;
	QString	name;
	
	bool equalTo(Country* other) {
		if (name != other->name)			return false;
		return true;
	}
};



#endif // COUNTRY_H
