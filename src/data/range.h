#ifndef RANGE_H
#define RANGE_H

#include <QString>
#include <QMap>
#include <QTranslator>



class Range
{
public:
	int		rangeID;
	QString	name;
	int*	continent;
	
	static QString continentNames[];	// = {QObject::tr("North America"), QObject::tr("South America"), QObject::tr("Europe"), QObject::tr("Africa"), QObject::tr("Asia"), QObject::tr("Australia"), QObject::tr("Antarctica")};
};



#endif // RANGE_H
