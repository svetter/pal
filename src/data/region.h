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
	
	bool equalTo(Ascent* other) {
		if (name != other->name)			return false;
		if (countryID != other->countryID)	return false;
		return true;
	}
};



#endif // REGION_H
