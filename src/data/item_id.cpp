/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "item_id.h"

#include <QHashFunctions>
#include <assert.h>



const int ItemID::LOWEST_LEGAL_ID = 1;



ItemID::ItemID(int id) : valid(id >= LOWEST_LEGAL_ID), id(id)
{}

ItemID::ItemID(QVariant id) : valid(id.isValid() && id.canConvert<int>() && id.toInt() >= LOWEST_LEGAL_ID), id(id.toInt())
{}

ItemID::ItemID() : valid(false), id(LOWEST_LEGAL_ID - 1)
{}



bool ItemID::isValid() const
{
	return valid;
}

bool ItemID::isInvalid() const
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
