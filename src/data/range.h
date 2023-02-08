#ifndef RANGE_H
#define RANGE_H

#include <QStringList>
#include <QTranslator>



class Range
{
public:
	int		rangeID;
	QString	name;
	int		continent;
	
	static const QStringList continentNames;
	
	Range(int rangeID, QString& name, int continent);
	
	bool equalTo(Range* other);
};



#endif // RANGE_H
