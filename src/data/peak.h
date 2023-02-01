#ifndef PEAK_H
#define PEAK_H

#include "src/data/region.h"

#include <QString>



class Peak
{
public:
	int		peakID;
	QString	name;
	int		height;
	bool	volcano;
	int		regionID;
	QString	mapsLink;
	QString	earthLink;
	QString	wikiLink;
};



#endif // PEAK_H
