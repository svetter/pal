#ifndef RANGE_H
#define RANGE_H

#include "item_id.h"

#include <QStringList>
#include <QTranslator>



class Range : private QObject
{
	Q_OBJECT
	
public:
	ItemID	rangeID;
	QString	name;
	int		continent;
	
	static const QStringList continentNames;
	
	Range(ItemID rangeID, QString& name, int continent);
	virtual ~Range();
	
	bool equalTo(const Range* const other) const;
};



#endif // RANGE_H
