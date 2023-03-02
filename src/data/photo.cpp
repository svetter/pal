#include "photo.h"



Photo::Photo() :
		photoID(ItemID()),
		ascentID(ItemID()),
		sortIndex(-1),
		filepath(QString()),
		description(QString())
{}

Photo::Photo(ItemID photoID, ItemID ascentID, int sortIndex, QString filepath, QString description) :
		photoID(photoID),
		ascentID(ascentID),
		sortIndex(sortIndex),
		filepath(filepath),
		description(description)
{}

Photo::Photo(const Photo& originalPhoto) :
		photoID(originalPhoto.photoID),
		ascentID(originalPhoto.ascentID),
		sortIndex(originalPhoto.sortIndex),
		filepath(originalPhoto.filepath),
		description(originalPhoto.description)
{}



Photo& Photo::operator=(const Photo& other)
{
	ascentID	= other.ascentID;
	filepath	= other.filepath;
	description	= other.description;
	return *this;
}

bool Photo::operator==(const Photo& other) const
{
	if (ascentID	!= other.ascentID)		return false;
	if (filepath	!= other.filepath)		return false;
	if (description	!= other.description)	return false;
	return true;
}
