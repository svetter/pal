#ifndef PEAK_H
#define PEAK_H

#include "item_id.h"

#include <QString>



class Peak
{
public:
	ItemID	peakID;
	QString	name;
	int		height;
	bool	volcano;
	ItemID	regionID;
	QString	mapsLink;
	QString	earthLink;
	QString	wikiLink;
	
	Peak(ItemID peakID, QString& name, int height, bool volcano, ItemID regionID, QString& mapsLink, QString& earthLink, QString& wikiLink);
	
	bool equalTo(const Peak* const other) const;
	
	bool heightSpecified() const;
};



#endif // PEAK_H
