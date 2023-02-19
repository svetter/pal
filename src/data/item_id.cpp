#include "item_id.h"

#include <QHashFunctions>
#include <assert.h>



const int ItemID::LOWEST_LEGAL_ID = 1;



ItemID::ItemID(int id) : valid(id >= LOWEST_LEGAL_ID), id(id)
{}

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

ValidItemID ItemID::forceValid() const
{
	assert(valid);
	return ValidItemID(id);
}





ValidItemID::ValidItemID(int id) : ItemID(id)
{
	assert(isValid());
}
