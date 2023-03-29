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

#ifndef PEAK_H
#define PEAK_H

#include "item_id.h"

#include <QString>



class Peak : private QObject
{
	Q_OBJECT
	
public:
	ItemID	peakID;
	QString	name;
	int		height;
	bool	volcano;
	ItemID	regionID;
	QString	mapsLink;
	QString	earthLink;
	QString	wikiLink;
	
	Peak(ItemID peakID, QString& name, int height, bool volcano, ItemID regionID, QString& mapsLink, QString& earthLink, QString& wikiLink);
	virtual ~Peak();
	
	bool equalTo(const Peak* const other) const;
	
	bool heightSpecified() const;
	
	QVariant getHeightAsQVariant() const;
};



#endif // PEAK_H
