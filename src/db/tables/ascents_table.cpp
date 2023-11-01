/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "ascents_table.h"

#include <QString>
#include <QTranslator>



AscentsTable::AscentsTable(Column* foreignPeakIDColumn,  Column* foreignTripIDColumn) :
		NormalTable(QString("Ascents"), tr("Ascents"), "ascentID"),
		//									name				uiName				type		nullable	primaryKey	foreignKey				inTable
		titleColumn				(new Column("title",			tr("Title"),		String,		true,		false,		nullptr,				this)),
		peakIDColumn			(new Column("peakID",			QString(),			ID,			true,		false,		foreignPeakIDColumn,	this)),
		dateColumn				(new Column("date",				tr("Date"),			Date,		true,		false,		nullptr,				this)),
		peakOnDayColumn			(new Column("peakOnDay",		tr("Peak/day"),		Integer,	false,		false,		nullptr,				this)),
		timeColumn				(new Column("time",				tr("Local time"),	Time,		true,		false,		nullptr,				this)),
		elevationGainColumn		(new Column("elevationGain",	tr("Elev. gain"),	Integer,	true,		false,		nullptr,				this)),
		hikeKindColumn			(new Column("hikeKind",			tr("Kind of hike"),	Enum,		false,		false,		nullptr,				this)),
		traverseColumn			(new Column("traverse",			tr("Traverse"),		Bit,		false,		false,		nullptr,				this)),
		difficultySystemColumn	(new Column("difficultySystem",	tr("Diff. system"),	DualEnum,	false,		false,		nullptr,				this)),
		difficultyGradeColumn	(new Column("difficultyGrade",	tr("Diff. grade"),	DualEnum,	false,		false,		nullptr,				this)),
		tripIDColumn			(new Column("tripID",			QString(),			ID,			true,		false,		foreignTripIDColumn,	this)),
		descriptionColumn		(new Column("description",		tr("Description"),	String,		true,		false,		nullptr,				this))
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



BufferRowIndex AscentsTable::addRow(QWidget* parent, Ascent* ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, ascent);
	
	BufferRowIndex newAscentIndex = NormalTable::addRow(parent, columns, data);
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
