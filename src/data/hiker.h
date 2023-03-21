#ifndef HIKER_H
#define HIKER_H

#include "item_id.h"

#include <QString>



class Hiker : private QObject
{
	Q_OBJECT
	
public:
	ItemID	hikerID;
	QString	name;
	
	Hiker(ItemID hikerID, QString& name);
	virtual ~Hiker();
	
	bool equalTo(const Hiker* const other) const;
};



#endif // HIKER_H
