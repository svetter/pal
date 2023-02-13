#ifndef REGION_H
#define REGION_H

#include "country.h"

#include <QString>



class Region
{
public:
	int		regionID;
	QString	name;
	int		rangeID;
	int		countryID;
	
	Region(int regionID, QString& name, int rangeID, int countryID);
	
	bool equalTo(const Region* const other) const;
};



#endif // REGION_H
