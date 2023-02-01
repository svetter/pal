#ifndef PHOTO_H
#define PHOTO_H

#include "src/data/region.h"

#include <QString>



class Peak
{
public:
	int			peakID;
	QString		name;
	int*		height;
	bool		volcano;
	Region*		region;
	QString*	mapsLink;
	QString*	earthLink;
	QString*	wikiLink;
};



#endif // PHOTO_H
