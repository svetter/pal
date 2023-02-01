#ifndef REGION_H
#define REGION_H

#include "country.h"

#include <QString>



class Region
{
public:
	int			regionID;
	QString*	name;
	Country*	country;
};



#endif // REGION_H
