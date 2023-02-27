#ifndef PHOTO_H
#define PHOTO_H

#include "item_id.h"

#include <QString>



class Photo
{
public:
	ItemID	photoID;
	ItemID	ascentID;
	int		sortIndex;
	bool	useBasePath;
	QString	filepath;
	QString	description;
	
	Photo();
	Photo(ItemID photoID, ItemID ascentID, int sortIndex, bool useBasePath, QString& filepath, QString description);
	Photo(const Photo& originalPhoto);
	
	Photo& operator=(const Photo& other);
	
	bool operator==(const Photo& other) const;
};



#endif // PHOTO_H
