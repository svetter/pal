#ifndef REGION_H
#define REGION_H

#include "item_id.h"

#include <QString>



class Region : private QObject
{
	Q_OBJECT
	
public:
	ItemID	regionID;
	QString	name;
	ItemID	rangeID;
	ItemID	countryID;
	
	Region(ItemID regionID, QString& name, ItemID rangeID, ItemID countryID);
	virtual ~Region();
	
	bool equalTo(const Region* const other) const;
};



#endif // REGION_H
