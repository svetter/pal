#ifndef ASCENT_H
#define ASCENT_H

#include "src/data/peak.h"
#include "src/data/trip.h"
#include "src/data/hiker.h"

#include <QString>
#include <QList>
#include <QDate>



class Ascent
{
public:
	int				ascentID;
	QString*		title;
	Peak*			peak;
	QDate*			date;
	int				perDayIndex;
	QTime*			time;
	int				hikeKind;
	bool			traverse;
	int*			difficultySystem;
	int*			difficultyGrade;
	Trip*			trip;
	QList<Hiker*>	hikers;
	QList<QString>	photos;
	QString*		notes;
	
	static QString hikeKindNames[];
	static QString difficultySystemNames[];
	static QString difficultyGradeNames[];
};



#endif // ASCENT_H
