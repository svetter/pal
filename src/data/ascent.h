#ifndef ASCENT_H
#define ASCENT_H

#include <QString>
#include <QList>
#include <QDate>



class Ascent
{
public:
	int				ascentID;
	QString			title;
	int				peakID;
	QDate			date;
	int				perDayIndex;
	QTime			time;
	int				hikeKind;
	bool			traverse;
	int				difficultySystem;
	int				difficultyGrade;
	int				tripID;
	QList<int>		hikerIDs;
	QList<QString>	photos;
	QString			notes;
	
	static QString hikeKindNames[];
	static QString* difficultySystemNames;
	static QString** difficultyGradeNames;
	
	bool equalTo(Ascent* other) {
		if (title != other->title)							return false;
		if (peakID != other->peakID)						return false;
		if (date != other->date)							return false;
		if (perDayIndex != other->perDayIndex)				return false;
		if (time != other->time)							return false;
		if (hikeKind != other->hikeKind)					return false;
		if (traverse != other->traverse)					return false;
		if (difficultySystem != other->difficultySystem)	return false;
		if (difficultyGrade != other->difficultyGrade)		return false;
		if (tripID != other->tripID)						return false;
		if (hikerIDs != other->hikerIDs)					return false;
		if (photos != other->photos)						return false;
		if (notes != other->notes)							return false;
		return true;
	}
};



#endif // ASCENT_H
