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

#include "ascent.h"

#include <QCoreApplication>



Ascent::Ascent(ItemID ascentID, QString& title, ItemID peakID, QDate& date, int perDayIndex, QTime& time, int elevationGain, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, ItemID tripID, QSet<ValidItemID>& hikerIDs, QList<Photo>& photos, QString& description) :
		ascentID(ascentID),
		title(title),
		peakID(peakID),
		date(date),
		perDayIndex(perDayIndex),
		time(time),
		elevationGain(elevationGain),
		hikeKind(hikeKind),
		traverse(traverse),
		difficultySystem(difficultySystem),
		difficultyGrade(difficultyGrade),
		tripID(tripID),
		hikerIDs(hikerIDs),
		photos(photos),
		description(description)
{}

Ascent::~Ascent()
{}



bool Ascent::equalTo(const Ascent* const other) const
{
	assert(other);
	if (title				!= other->title)			return false;
	if (peakID				!= other->peakID)			return false;
	if (date				!= other->date)				return false;
	if (perDayIndex			!= other->perDayIndex)		return false;
	if (time				!= other->time)				return false;
	if (elevationGain		!= other->elevationGain)	return false;
	if (hikeKind			!= other->hikeKind)			return false;
	if (traverse			!= other->traverse)			return false;
	if (difficultySystem	!= other->difficultySystem)	return false;
	if (difficultyGrade		!= other->difficultyGrade)	return false;
	if (tripID				!= other->tripID)			return false;
	if (hikerIDs			!= other->hikerIDs)			return false;
	if (photos				!= other->photos)			return false;
	if (description			!= other->description)		return false;
	return true;
}



bool Ascent::dateSpecified() const
{
	return date.isValid();
}

bool Ascent::timeSpecified() const
{
	return time.isValid();
}

bool Ascent::elevationGainSpecified() const
{
	return elevationGain >= 0;
}



QVariant Ascent::getElevationGainAsQVariant() const
{
	if (!elevationGainSpecified()) return QVariant();
	return elevationGain;
}
