#ifndef PHOTO_H
#define PHOTO_H

#include "item_id.h"

#include <QString>



class Photo : private QObject
{
	Q_OBJECT
	
public:
	ItemID	photoID;
	ItemID	ascentID;
	int		sortIndex;
	QString	filepath;
	QString	description;
	
	Photo();
	Photo(ItemID photoID, ItemID ascentID, int sortIndex, QString filepath, QString description);
	Photo(const Photo& originalPhoto);
	virtual ~Photo();
	
	Photo& operator=(const Photo& other);
	
	bool operator==(const Photo& other) const;
};



#endif // PHOTO_H
