#ifndef PEAK_H
#define PEAK_H

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
	
	Peak(int peakID, QString& name, int height, bool volcano, int regionID, QString& mapsLink, QString& earthLink, QString& wikiLink);
	
	bool equalTo(Peak* other);
};



#endif // PEAK_H
