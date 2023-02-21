#ifndef PHOTO_H
#define PHOTO_H

#include "item_id.h"

#include <QString>



class Photo
{
public:
	ItemID	photoID;
	ItemID	ascentID;
	bool	useBasePath;
	QString	filepath;
	QString	description;
	
	Photo(ItemID photoID, ItemID ascentID, bool useBasePath, QString& filepath, QString& description);
	
	bool equalTo(const Photo* const other) const;
};



#endif // PHOTO_H
