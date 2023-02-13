#include "ascents_table.h"

#include <QString>
#include <QTranslator>



AscentsTable::AscentsTable(Column* foreignPeakIDColumn, Column* foreignTripIDColumn) :
		NormalTable(QString("Ascents"), QString("ascent"), tr("Ascents"), tr("None")),
		//									name							uiName					type		nullable	primaryKey	foreignKey				inTable
		titleColumn				(new Column(QString("title"),				tr("Title"),			varchar,	true,		false,		nullptr,				this)),
		peakIDColumn			(new Column(QString("peakID"),				QString(),				integer,	true,		false,		foreignPeakIDColumn,	this)),
		dateColumn				(new Column(QString("date"),				tr("Date"),				date,		true,		false,		nullptr,				this)),
		peakOnDayColumn			(new Column(QString("peakOnDay"),			tr("Peak of the day"),	integer,	false,		false,		nullptr,				this)),
		timeColumn				(new Column(QString("time"),				tr("Time"),				time_,		true,		false,		nullptr,				this)),
		hikeKindColumn			(new Column(QString("hikeKind"),			tr("Kind of hike"),		integer,	false,		false,		nullptr,				this)),
		traverseColumn			(new Column(QString("traverse"),			tr("Traverse"),			bit,		false,		false,		nullptr,				this)),
		difficultySystemColumn	(new Column(QString("difficultySystem"),	tr("Diff. system"),		integer,	true,		false,		nullptr,				this)),
		difficultyGradeColumn	(new Column(QString("difficultyGrade"),		tr("Diff. grade"),		integer,	true,		false,		nullptr,				this)),
		tripIDColumn			(new Column(QString("tripID"),				QString(),				integer,	true,		false,		foreignTripIDColumn,	this)),
		descriptionColumn		(new Column(QString("description"),			tr("Description"),		varchar,	true,		false,		nullptr,				this))
{
	addColumn(titleColumn);
	addColumn(peakIDColumn);
	addColumn(dateColumn);
	addColumn(peakOnDayColumn);
	addColumn(timeColumn);
	addColumn(hikeKindColumn);
	addColumn(traverseColumn);
	addColumn(difficultySystemColumn);
	addColumn(difficultyGradeColumn);
	addColumn(tripIDColumn);
	addColumn(descriptionColumn);
}



int AscentsTable::addRow(QWidget* parent, const Ascent* ascent)
{
	QList<Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == titleColumn)				{ data.append(ascent->title);				continue; }
		if (*iter == peakIDColumn)				{ data.append(ascent->peakID);				continue; }
		if (*iter == dateColumn)				{ data.append(ascent->date);				continue; }
		if (*iter == peakOnDayColumn)			{ data.append(ascent->perDayIndex);			continue; }
		if (*iter == timeColumn)				{ data.append(ascent->time);				continue; }
		if (*iter == hikeKindColumn)			{ data.append(ascent->hikeKind);			continue; }
		if (*iter == traverseColumn)			{ data.append(ascent->traverse);			continue; }
		if (*iter == difficultySystemColumn)	{ data.append(ascent->difficultySystem);	continue; }
		if (*iter == difficultyGradeColumn)		{ data.append(ascent->difficultyGrade);		continue; }
		if (*iter == tripIDColumn)				{ data.append(ascent->tripID);				continue; }
		if (*iter == descriptionColumn)			{ data.append(ascent->description);			continue; }
		assert(false);
	}
	int newAscentIndex = NormalTable::addRow(parent, data);
	return newAscentIndex;
}