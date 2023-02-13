#ifndef COUNTRY_H
#define COUNTRY_H

#include <QString>



class Country
{
public:
	int		countryID;
	QString	name;
	
	Country(int countryID, QString& name);
	
	bool equalTo(const Country* const other) const;
};



#endif // COUNTRY_H
