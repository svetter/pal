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

/**
 * @file ascent.cpp
 * 
 * This file defines the internal representation of an ascent item.
 */

#include "ascent.h"

#include <QCoreApplication>



/**
 * Creates a new ascent object with the given properties.
 * 
 * @param ascentID			The ID of the ascent item, if it already has one. Invalid ItemID otherwise.
 * @param title				The title of the ascent, if specified. Empty QString otherwise.
 * @param peakID			The ID of the scaled peak, if specified. Invalid ItemID otherwise.
 * @param date				The date of the ascent, if specified. Invalid QDate otherwise.
 * @param perDayIndex		The index of the ascent in the day it was made on, if specified. -1 otherwise.
 * @param time				The time the ascent was made at, if specified. Invalid QTime otherwise.
 * @param elevationGain		The elevation gain of the ascent, if specified. -1 otherwise.
 * @param hikeKind			The kind of hike the ascent was made on, if specified. -1 otherwise.
 * @param traverse			Whether the ascent was a traverse. Cannot be unspecified.
 * @param difficultySystem	The difficulty system used to grade the ascent, if specified. -1 otherwise.
 * @param difficultyGrade	The difficulty grade of the ascent, if specified. -1 otherwise.
 * @param tripID			The ID of the trip which the ascent was a part of, if specified. Invalid ItemID otherwise.
 * @param hikerIDs			The IDs of the hikers that participated, if specified. Empty QSet otherwise.
 * @param photos			The photos of the ascent, if any. Empty QList otherwise.
 * @param description		The description of the ascent, if specified. Empty QString otherwise.
 */
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

/**
 * Destroys the ascent object.
 */
Ascent::~Ascent()
{}



/**
 * Checks the given ascent object for equality with this one.
 * 
 * @param other	The other ascent.
 * @return		True if the other ascent is equal to this one, false otherwise.
 */
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



/**
 * Checks whether the ascent's date is specified.
 * 
 * @return	True if the ascent's date is specified, false otherwise.
 */
bool Ascent::dateSpecified() const
{
	return date.isValid();
}

/**
 * Checks whether the ascent's time is specified.
 * 
 * @return	True if the ascent's time is specified, false otherwise.
 */
bool Ascent::timeSpecified() const
{
	return time.isValid();
}

/**
 * Checks whether the ascent's elevation gain is specified.
 * 
 * @return	True if the ascent's elevation gain is specified, false otherwise.
 */
bool Ascent::elevationGainSpecified() const
{
	return elevationGain >= 0;
}



/**
 * Returns the ascent's elevation gain as a QVariant.
 * 
 * If the ascent's elevation gain is not specified, an invalid QVariant is returned.
 * 
 * @return	A QVariant containing the ascent's elevation gain, or an invalid QVariant.
 */
QVariant Ascent::getElevationGainAsQVariant() const
{
	if (!elevationGainSpecified()) return QVariant();
	return elevationGain;
}
