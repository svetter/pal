#ifndef REGION_H
#define REGION_H

#include "item_id.h"

#include <QString>



class Region
{
public:
	ItemID	regionID;
	QString	name;
	ItemID	rangeID;
	ItemID	countryID;
	
	Region(ItemID regionID, QString& name, ItemID rangeID, ItemID countryID);
	
	bool equalTo(const Region* const other) const;
};



#endif // REGION_H
