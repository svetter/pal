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



class Ascent : private QObject
{
	Q_OBJECT
	
public:
	ItemID				ascentID;
	QString				title;
	ItemID				peakID;
	QDate				date;
	int					perDayIndex;
	QTime				time;
	int					elevationGain;
	int					hikeKind;
	bool				traverse;
	int					difficultySystem;
	int					difficultyGrade;
	ItemID				tripID;
	QSet<ValidItemID>	hikerIDs;
	QList<Photo>		photos;
	QString				description;
	
	static const QStringList hikeKindNames;
	static const QList<QPair<QString, QStringList>> difficultyNames;
	
	Ascent(ItemID ascentID, QString& title, ItemID peakID, QDate& date, int perDayIndex, QTime& time, int elevationGain, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, ItemID tripID, QSet<ValidItemID>& hikerIDs, QList<Photo>& photos, QString& description);
	virtual ~Ascent();
	
	bool equalTo(const Ascent* const other) const;
	
	bool dateSpecified() const;
	bool timeSpecified() const;
	bool elevationGainSpecified() const;
	
	QVariant getElevationGainAsQVariant() const;
};



#endif // ASCENT_H
