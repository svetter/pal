#include "item_id.h"

#include <QHashFunctions>
#include <assert.h>



const int ItemID::LOWEST_LEGAL_ID = 1;



ItemID::ItemID(int id) : valid(id >= LOWEST_LEGAL_ID), id(id)
{}

ItemID::ItemID(QVariant id) : valid(id.toInt() >= LOWEST_LEGAL_ID), id(id.toInt())
{
	assert(id.canConvert<int>());
}

ItemID::ItemID() : valid(false), id(LOWEST_LEGAL_ID - 1)
{}



bool ItemID::isValid() const
{
	return valid;
}

bool ItemID::isNull() const
{
	return !valid;
}

int ItemID::get() const
{
	assert(valid);
	return id;
}

QVariant ItemID::asQVariant() const
{
	if (isValid()) {
		return QVariant(id);
	} else {
		return QVariant();
	}
}

ValidItemID ItemID::forceValid() const
{
	assert(valid);
	return ValidItemID(id);
}





ValidItemID::ValidItemID(int id) : ItemID(id)
{
	assert(isValid());
}

ValidItemID::ValidItemID(QVariant id) : ItemID(id)
{
	assert(isValid());
}
