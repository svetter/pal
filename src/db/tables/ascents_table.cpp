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

/**
 * @file src/db/tables/ascents_table.h
 * 
 * This file defines the AscentsTable class.
 */

#include "ascents_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new AscentsTable.
 * 
 * @param foreignPeakIDColumn	The primary key column of the PeaksTable.
 * @param foreignTripIDColumn	The primary key column of the TripsTable.
 */
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



/**
 * Adds a new ascent to the table.
 * 
 * @param parent	The parent widget.
 * @param ascent	The ascent to add.
 * @return			The index of the new ascent in the table buffer.
 */
BufferRowIndex AscentsTable::addRow(QWidget* parent, Ascent* ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, ascent);
	
	BufferRowIndex newAscentIndex = NormalTable::addRow(parent, columns, data);
	ascent->ascentID = getPrimaryKeyAt(newAscentIndex);
	return newAscentIndex;
}

/**
 * Updates the contents of an existing ascent in the table.
 * 
 * @pre The given ascent's ItemID will be used to identify the ascent to update and thus must be valid.
 * 
 * @param parent	The parent widget.
 * @param ascent	The ascent to update.
 */
void AscentsTable::updateRow(QWidget* parent, const Ascent* ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, ascent);
	
	NormalTable::updateRow(parent, ascent->ascentID.forceValid(), columns, data);
}


/**
 * Translates the data of an ascent to a list of QVariants.
 * 
 * @param columns	The column list specifying the order of the data.
 * @param ascent	The ascent from which to get the data.
 * @return			The list of QVariants representing the ascent's data.
 */
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



/**
 * Returns the translated string to be displayed in a combo box containing ascents when none is
 * selected.
 *
 * @return	The translated string representing absence of an ascent.
 */
QString AscentsTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns the translation of "ascent" (singular) in lowercase.
 *
 * @return	The translation of "ascent" (singular) in lowercase.
 */
QString AscentsTable::getItemNameSingularLowercase() const
{
	return tr("ascent");
}

/**
 * Returns the translation of "ascents" (plural) in lowercase.
 *
 * @return	The translation of "ascents" (plural) in lowercase.
 */
QString AscentsTable::getItemNamePluralLowercase() const
{
	return tr("ascents");
}
