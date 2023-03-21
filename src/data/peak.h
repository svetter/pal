#ifndef PEAK_H
#define PEAK_H

#include "item_id.h"

#include <QString>



class Peak : private QObject
{
	Q_OBJECT
	
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
	virtual ~Peak();
	
	bool equalTo(const Peak* const other) const;
	
	bool heightSpecified() const;
	
	QVariant getHeightAsQVariant() const;
};



#endif // PEAK_H
