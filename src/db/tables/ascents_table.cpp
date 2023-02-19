#include "ascents_table.h"

#include <QString>
#include <QTranslator>



AscentsTable::AscentsTable(const Column* foreignPeakIDColumn, const Column* foreignTripIDColumn) :
		NormalTable(QString("Ascents"), tr("Ascents"), "ascentID"),
		//												name					uiName				type		nullable	primaryKey	foreignKey				inTable
		titleColumn				(new const Column(QString("title"),				tr("Title"),		varchar,	true,		false,		nullptr,				this)),
		peakIDColumn			(new const Column(QString("peakID"),			QString(),			integer,	true,		false,		foreignPeakIDColumn,	this)),
		dateColumn				(new const Column(QString("date"),				tr("Date"),			date,		true,		false,		nullptr,				this)),
		peakOnDayColumn			(new const Column(QString("peakOnDay"),			tr("Peak/day"),		integer,	false,		false,		nullptr,				this)),
		timeColumn				(new const Column(QString("time"),				tr("Time"),			time_,		true,		false,		nullptr,				this)),
		elevationGainColumn		(new const Column(QString("elevationGain"),		tr("Elev. gain"),	integer,	true,		false,		nullptr,				this)),
		hikeKindColumn			(new const Column(QString("hikeKind"),			tr("Kind of hike"),	integer,	false,		false,		nullptr,				this)),
		traverseColumn			(new const Column(QString("traverse"),			tr("Traverse"),		bit,		false,		false,		nullptr,				this)),
		difficultySystemColumn	(new const Column(QString("difficultySystem"),	tr("Diff. system"),	integer,	false,		false,		nullptr,				this)),
		difficultyGradeColumn	(new const Column(QString("difficultyGrade"),	tr("Diff. grade"),	integer,	false,		false,		nullptr,				this)),
		tripIDColumn			(new const Column(QString("tripID"),			QString(),			integer,	true,		false,		foreignTripIDColumn,	this)),
		descriptionColumn		(new const Column(QString("description"),		tr("Description"),	varchar,	true,		false,		nullptr,				this))
{
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
	assert(ascent->ascentID == -1);
	QList<QVariant> data = mapDataToQVariantList(ascent);
	
	int newAscentIndex = NormalTable::addRow(parent, data);
	ascent->ascentID = buffer->at(newAscentIndex)->at(getPrimaryKeyColumn()->getIndex()).toInt();
	return newAscentIndex;
}

void AscentsTable::updateRow(QWidget* parent, ValidItemID ascentsID, const Ascent* ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(ascent);
	
	NormalTable::updateRow(parent, ascentsID, columns, data);
}


QList<QVariant> AscentsTable::mapDataToQVariantList(const Ascent* ascent) const
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == titleColumn)				{ data.append(ascent->title);				continue; }
		if (column == peakIDColumn)				{ data.append(ascent->peakID.asQVariant());	continue; }
		if (column == dateColumn)				{ data.append(ascent->date);				continue; }
		if (column == peakOnDayColumn)			{ data.append(ascent->perDayIndex);			continue; }
		if (column == timeColumn)				{ data.append(ascent->time);				continue; }
		if (column == elevationGainColumn)		{ data.append(ascent->elevationGain);		continue; }
		if (column == hikeKindColumn)			{ data.append(ascent->hikeKind);			continue; }
		if (column == traverseColumn)			{ data.append(ascent->traverse);			continue; }
		if (column == difficultySystemColumn)	{ data.append(ascent->difficultySystem);	continue; }
		if (column == difficultyGradeColumn)	{ data.append(ascent->difficultyGrade);		continue; }
		if (column == tripIDColumn)				{ data.append(ascent->tripID.asQVariant());	continue; }
		if (column == descriptionColumn)		{ data.append(ascent->description);			continue; }
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
