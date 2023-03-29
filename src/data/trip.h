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

#ifndef TRIP_H
#define TRIP_H

#include "item_id.h"

#include <QString>
#include <QDate>



class Trip : private QObject
{
	Q_OBJECT
	
public:
	ItemID	tripID;
	QString	name;
	QDate	startDate;
	QDate	endDate;
	QString	description;
	
	Trip(ItemID tripID, QString& name, QDate& startDate, QDate& endDate, QString& description);
	virtual ~Trip();
	
	bool equalTo(const Trip* const other) const;
	
	bool datesSpecified() const;
};



#endif // TRIP_H
