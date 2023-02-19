#ifndef RANGE_H
#define RANGE_H

#include "item_id.h"

#include <QStringList>
#include <QTranslator>



class Range
{
public:
	ItemID	rangeID;
	QString	name;
	int		continent;
	
	static const QStringList continentNames;
	
	Range(ItemID rangeID, QString& name, int continent);
	
	bool equalTo(const Range* const other) const;
	
private:
	static QString tr(const char* string);
};



#endif // RANGE_H
