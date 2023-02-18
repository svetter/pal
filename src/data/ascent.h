#ifndef ASCENT_H
#define ASCENT_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QSet>
#include <QPair>
#include <QDate>



class Ascent
{
public:
	int			ascentID;
	QString		title;
	int			peakID;
	QDate		date;
	int			perDayIndex;
	QTime		time;
	int			elevationGain;
	int			hikeKind;
	bool		traverse;
	int			difficultySystem;
	int			difficultyGrade;
	int			tripID;
	QSet<int>	hikerIDs;
	QStringList	photos;
	QString		description;
	
	static const QStringList hikeKindNames;
	static const QList<QPair<QString, QStringList>> difficultyNames;
	
	Ascent(int ascentID, QString& title, int peakID, QDate& date, int perDayIndex, QTime& time, int elevationGain, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, int tripID, QSet<int>& hikerIDs, QStringList& photos, QString& description);
	
	bool equalTo(const Ascent* const other) const;
	
	bool dateSpecified() const;
	bool timeSpecified() const;
	bool elevationGainSpecified() const;
	
private:
	static QString tr(const char* string);
};



#endif // ASCENT_H
