/*
 * Copyright 2023-2025 Simon Vetter
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

/**
 * @file ascent.h
 * 
 * This file declares the internal representation of an ascent item.
 */

#ifndef ASCENT_H
#define ASCENT_H

#include "item_id.h"
#include "src/data/photo.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QSet>
#include <QPair>
#include <QDate>



/**
 * A class representing an ascent item.
 */
class Ascent : private QObject
{
	Q_OBJECT
	
public:
	/** The ascent's internal ID. Invalid ItemID if this is a new item. */
	ItemID				ascentID;
	/** The ascent's title. Empty QString if not specified. */
	QString				title;
	/** The ID of the peak that was ascended. Invalid ItemID if not specified. */
	ItemID				peakID;
	/** The date on which the peak was reached. Invalid QDate if not specified. */
	QDate				date;
	/** The index of the ascent within the day (*n*th ascent of the day). -1 if not specified. */
	int					perDayIndex;
	/** The time at which the peak was reached. Invalid QTime if not specified. */
	QTime				time;
	/** The elevation gain of the ascent in meters. -1 if not specified. */
	int					elevationGain;
	/** The kind of hike. Refers to values in EnumNames::hikeKindNames. -1 if not specified. */
	int					hikeKind;
	/** Whether the peak was traversed on the ascent. Cannot be unspecified. */
	bool				traverse;
	/** The difficulty system used to grade the ascent. Refers to values in EnumNames::difficultySystemNames. -1 if not specified. */
	int					difficultySystem;
	/** The difficulty grade of the ascent. Refers to values in EnumNames::difficultyGradeNames. -1 if not specified. */
	int					difficultyGrade;
	/** The ID of the trip during which the ascent was made. Invalid ItemID if not specified. */
	ItemID				tripID;
	/** The IDs of the hikers who participated in the ascent. Empty QSet if not specified. */
	QSet<ValidItemID>	hikerIDs;
	/** The photos linked to the ascent. Empty QList if not specified. */
	QList<Photo>		photos;
	/** The ascent's description. Empty QString if not specified. */
	QString				description;
	
	Ascent(ItemID ascentID, QString& title, ItemID peakID, QDate& date, int perDayIndex, QTime& time, int elevationGain, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, ItemID tripID, QSet<ValidItemID>& hikerIDs, QList<Photo>& photos, QString& description);
	virtual ~Ascent();
	
	bool equalTo(const Ascent& other) const;
	
	bool dateSpecified() const;
	bool timeSpecified() const;
	bool elevationGainSpecified() const;
	
	QVariant getElevationGainAsQVariant() const;
};



#endif // ASCENT_H
