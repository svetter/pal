#ifndef REGION_H
#define REGION_H

#include "country.h"

#include <QString>



class Region
{
public:
	int		regionID;
	QString	name;
	int		countryID;
	
	Region(int regionID, QString& name, int countryID);
	
	bool equalTo(Region* other);
};



#endif // REGION_H
