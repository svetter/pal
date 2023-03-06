#include "ascents_table.h"

#include <QString>
#include <QTranslator>



AscentsTable::AscentsTable(Column* foreignPeakIDColumn,  Column* foreignTripIDColumn) :
		NormalTable(QString("Ascents"), tr("Ascents"), "ascentID"),
		//									name				uiName				type		nullable	primaryKey	foreignKey				inTable
		titleColumn				(new Column("title",			tr("Title"),		varchar,	true,		false,		nullptr,				this)),
		peakIDColumn			(new Column("peakID",			QString(),			integer,	true,		false,		foreignPeakIDColumn,	this)),
		dateColumn				(new Column("date",				tr("Date"),			date,		true,		false,		nullptr,				this)),
		peakOnDayColumn			(new Column("peakOnDay",		tr("Peak/day"),		integer,	false,		false,		nullptr,				this)),
		timeColumn				(new Column("time",				tr("Time"),			time_,		true,		false,		nullptr,				this)),
		elevationGainColumn		(new Column("elevationGain",	tr("Elev. gain"),	integer,	true,		false,		nullptr,				this)),
		hikeKindColumn			(new Column("hikeKind",			tr("Kind of hike"),	integer,	false,		false,		nullptr,				this)),
		traverseColumn			(new Column("traverse",			tr("Traverse"),		bit,		false,		false,		nullptr,				this)),
		difficultySystemColumn	(new Column("difficultySystem",	tr("Diff. system"),	integer,	false,		false,		nullptr,				this)),
		difficultyGradeColumn	(new Column("difficultyGrade",	tr("Diff. grade"),	integer,	false,		false,		nullptr,				this)),
		tripIDColumn			(new Column("tripID",			QString(),			integer,	true,		false,		foreignTripIDColumn,	this)),
		descriptionColumn		(new Column("description",		tr("Description"),	varchar,	true,		false,		nullptr,				this))
{
	addColumn(primaryKeyColumn);
	addColumn(titleColumn);
	addColumn(peakIDColumn);
	addColumn(dateColumn);
	addColumn(peakOnDayColumn);
	addColumn(timeColumn);
	addColumn(elevationGainColumn);
	addColumn(hikeKindColumn);
	addColumn(traverseColumn);
	addColumn(difficultySystemColumn);
	addColumn(difficultyGradeColumn);
	addColumn(tripIDColumn);
	addColumn(descriptionColumn);
}



int AscentsTable::addRow(QWidget* parent, Ascent* ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, ascent);
	
	int newAscentIndex = NormalTable::addRow(parent, columns, data);
	ascent->ascentID = getPrimaryKeyAt(newAscentIndex);
	return newAscentIndex;
}

void AscentsTable::updateRow(QWidget* parent, const Ascent* ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, ascent);
	
	NormalTable::updateRow(parent, ascent->ascentID.forceValid(), columns, data);
}


QList<QVariant> AscentsTable::mapDataToQVariantList(QList<const Column*>& columns, const Ascent* ascent) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == titleColumn)				{ data.append(ascent->title);							continue; }
		if (column == peakIDColumn)				{ data.append(ascent->peakID.asQVariant());				continue; }
		if (column == dateColumn)				{ data.append(ascent->date);							continue; }
		if (column == peakOnDayColumn)			{ data.append(ascent->perDayIndex);						continue; }
		if (column == timeColumn)				{ data.append(ascent->time);							continue; }
		if (column == elevationGainColumn)		{ data.append(ascent->getElevationGainAsQVariant());	continue; }
		if (column == hikeKindColumn)			{ data.append(ascent->hikeKind);						continue; }
		if (column == traverseColumn)			{ data.append(ascent->traverse);						continue; }
		if (column == difficultySystemColumn)	{ data.append(ascent->difficultySystem);				continue; }
		if (column == difficultyGradeColumn)	{ data.append(ascent->difficultyGrade);					continue; }
		if (column == tripIDColumn)				{ data.append(ascent->tripID.asQVariant());				continue; }
		if (column == descriptionColumn)		{ data.append(ascent->description);						continue; }
		assert(false);
	}
	return data;
}



QString AscentsTable::getNoneString() const
{
	return tr("None");
}

QString AscentsTable::getItemNameSingularLowercase() const
{
	return tr("ascent");
}

QString AscentsTable::getItemNamePluralLowercase() const
{
	return tr("ascents");
}
