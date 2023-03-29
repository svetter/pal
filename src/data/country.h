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

#ifndef COUNTRY_H
#define COUNTRY_H

#include "item_id.h"

#include <QString>



class Country : private QObject
{
	Q_OBJECT
	
public:
	ItemID	countryID;
	QString	name;
	
	Country(ItemID countryID, QString& name);
	virtual ~Country();
	
	bool equalTo(const Country* const other) const;
};



#endif // COUNTRY_H
