#include "photo.h"



Photo::Photo() :
		photoID(ItemID()),
		ascentID(ItemID()),
		sortIndex(-1),
		useBasePath(false),
		filepath(QString()),
		description(QString())
{}

Photo::Photo(ItemID photoID, ItemID ascentID, int sortIndex, bool useBasePath, QString& filepath, QString& description) :
		photoID(photoID),
		ascentID(ascentID),
		sortIndex(sortIndex),
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
