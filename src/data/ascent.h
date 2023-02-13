#ifndef ASCENT_H
#define ASCENT_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QPair>
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
	QString			description;
	
	static const QStringList hikeKindNames;
	static const QList<QPair<QString, QStringList>> difficultyNames;
	
	Ascent(int ascentID, QString& title, int peakID, QDate& date, int perDayIndex, QTime& time, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, int tripID, QList<int>& hikerIDs, QList<QString>& photos, QString& description);
	
	bool equalTo(Ascent* other);
	
private:
	static QString tr(const char* string);
};



#endif // ASCENT_H
