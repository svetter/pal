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
	static QString difficultySystemNames[];
	static QString difficultyGradeNames[];
};



#endif // ASCENT_H
