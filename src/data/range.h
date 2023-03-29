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

#ifndef RANGE_H
#define RANGE_H

#include "item_id.h"

#include <QStringList>
#include <QTranslator>



class Range : private QObject
{
	Q_OBJECT
	
public:
	ItemID	rangeID;
	QString	name;
	int		continent;
	
	static const QStringList continentNames;
	
	Range(ItemID rangeID, QString& name, int continent);
	virtual ~Range();
	
	bool equalTo(const Range* const other) const;
};



#endif // RANGE_H
