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
	int		continent;
	
	static QString continentNames[];
	
	Range(int rangeID, QString& name, int continent);
	
	bool equalTo(Range* other);
};



#endif // RANGE_H
