#include "photo.h"



Photo::Photo(ItemID photoID, ItemID ascentID, bool useBasePath, QString& filepath, QString& description) :
		photoID(photoID),
		ascentID(ascentID),
		useBasePath(useBasePath),
		filepath(filepath),
		description(description)
{}



bool Photo::equalTo(const Photo* const other) const
{
	assert(other);
	if (ascentID	!= other->ascentID)		return false;
	if (useBasePath	!= other->useBasePath)	return false;
	if (filepath	!= other->filepath)		return false;
	if (description	!= other->description)	return false;
	return true;
}
