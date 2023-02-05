#ifndef COUNTRY_H
#define COUNTRY_H

#include <QString>



class Country
{
public:
	int		countryID;
	QString	name;
	
	Country(int countryID, QString& name);
	
	bool equalTo(Country* other);
};



#endif // COUNTRY_H
