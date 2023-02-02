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
	
	bool equalTo(Ascent* other) {
		if (name != other->name)			return false;
		if (continent != other->continent)	return false;
		return true;
	}
};



#endif // RANGE_H
