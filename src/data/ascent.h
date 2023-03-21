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
