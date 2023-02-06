#include "ascents_table.h"

#include <QString>
#include <QTranslator>



AscentsTable::AscentsTable(Column* foreignPeakIDColumn, Column* foreignTripIDColumn) :
		NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
		//									name							uiName							type		nullable	primaryKey	foreignKey				inTable
		titleColumn				(new Column(QString("title"),				QObject::tr("Title"),			varchar,	true,		false,		nullptr,				this)),
		peakIDColumn			(new Column(QString("peakID"),				QString(),						integer,	true,		false,		foreignPeakIDColumn,	this)),
		dateColumn				(new Column(QString("date"),				QObject::tr("Date"),			date,		true,		false,		nullptr,				this)),
		peakOnDayColumn			(new Column(QString("peakOnDay"),			QObject::tr("Peak of the day"),	integer,	false,		false,		nullptr,				this)),
		timeColumn				(new Column(QString("time"),				QObject::tr("Time"),			time_,		true,		false,		nullptr,				this)),
		hikeKindColumn			(new Column(QString("hikeKind"),			QObject::tr("Kind of hike"),	integer,	false,		false,		nullptr,				this)),
		traverseColumn			(new Column(QString("traverse"),			QObject::tr("Traverse"),		bit,		false,		false,		nullptr,				this)),
		difficultySystemColumn	(new Column(QString("difficultySystem"),	QObject::tr("Diff. system"),	integer,	true,		false,		nullptr,				this)),
		difficultyGradeColumn	(new Column(QString("difficultyGrade"),		QObject::tr("Diff. grade"),		integer,	true,		false,		nullptr,				this)),
		tripIDColumn			(new Column(QString("tripID"),				QString(),						integer,	true,		false,		foreignTripIDColumn,	this)),
		notesColumn				(new Column(QString("notes"),				QObject::tr("Notes"),			varchar,	true,		false,		nullptr,				this))
{
	addColumn(titleColumn);
	addColumn(peakIDColumn);
	addColumn(dateColumn);
	addColumn(peakOnDayColumn);
	addColumn(timeColumn);
	addColumn(hikeKindColumn);
	addColumn(traverseColumn);
	addColumn(difficultySystemColumn);
	addColumn(difficultyGradeColumn	);
	addColumn(tripIDColumn);
	addColumn(notesColumn);
}



void AscentsTable::addRow(Ascent* ascent)
{
	// TODO
}