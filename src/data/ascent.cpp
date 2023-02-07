#include "ascent.h"



Ascent::Ascent(int ascentID, QString& title, int peakID, QDate& date, int perDayIndex, QTime& time, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, int tripID, QList<int>& hikerIDs, QList<QString>& photos, QString& notes) :
		ascentID(ascentID),
		title(title),
		peakID(peakID),
		date(date),
		perDayIndex(perDayIndex),
		time(time),
		hikeKind(hikeKind),
		traverse(traverse),
		difficultySystem(difficultySystem),
		difficultyGrade(difficultyGrade),
		tripID(tripID),
		hikerIDs(hikerIDs),
		photos(photos),
		notes(notes)
{}



bool Ascent::equalTo(Ascent* other)
{
	if (title				!= other->title)			return false;
	if (peakID				!= other->peakID)			return false;
	if (date				!= other->date)				return false;
	if (perDayIndex			!= other->perDayIndex)		return false;
	if (time				!= other->time)				return false;
	if (hikeKind			!= other->hikeKind)			return false;
	if (traverse			!= other->traverse)			return false;
	if (difficultySystem	!= other->difficultySystem)	return false;
	if (difficultyGrade		!= other->difficultyGrade)	return false;
	if (tripID				!= other->tripID)			return false;
	if (hikerIDs			!= other->hikerIDs)			return false;
	if (photos				!= other->photos)			return false;
	if (notes				!= other->notes)			return false;
	return true;
}