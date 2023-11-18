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
 * @file ascents_table.h
 * 
 * This file defines the AscentsTable class.
 */

#include "ascents_table.h"

#include "src/data/enum_names.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new AscentsTable.
 * 
 * @param foreignPeakIDColumn	The primary key column of the PeaksTable.
 * @param foreignTripIDColumn	The primary key column of the TripsTable.
 */
AscentsTable::AscentsTable(Column* foreignPeakIDColumn,  Column* foreignTripIDColumn) :
		NormalTable(QString("Ascents"), tr("Ascents"), "ascentID", tr("Ascent ID")),
		//									name				uiName				type		nullable	primaryKey	foreignKey				inTable	enumNames	enumNameLists
		titleColumn				(new Column("title",			tr("Title"),		String,		true,		false,		nullptr,				this)),
		peakIDColumn			(new Column("peakID",			tr("Peak ID"),		ID,			true,		false,		foreignPeakIDColumn,	this)),
		dateColumn				(new Column("date",				tr("Date"),			Date,		true,		false,		nullptr,				this)),
		peakOnDayColumn			(new Column("peakOnDay",		tr("Peak/day"),		Integer,	false,		false,		nullptr,				this)),
		timeColumn				(new Column("time",				tr("Local time"),	Time,		true,		false,		nullptr,				this)),
		elevationGainColumn		(new Column("elevationGain",	tr("Elev. gain"),	Integer,	true,		false,		nullptr,				this)),
		hikeKindColumn			(new Column("hikeKind",			tr("Kind of hike"),	Enum,		false,		false,		nullptr,				this,	&EnumNames::hikeKindNames)),
		traverseColumn			(new Column("traverse",			tr("Traverse"),		Bit,		false,		false,		nullptr,				this)),
		difficultySystemColumn	(new Column("difficultySystem",	tr("Diff. system"),	DualEnum,	false,		false,		nullptr,				this,	nullptr,	&EnumNames::difficultyNames)),
		difficultyGradeColumn	(new Column("difficultyGrade",	tr("Diff. grade"),	DualEnum,	false,		false,		nullptr,				this,	nullptr,	&EnumNames::difficultyNames)),
		tripIDColumn			(new Column("tripID",			tr("Trip ID"),		ID,			true,		false,		foreignTripIDColumn,	this)),
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
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, ascent);
	
	BufferRowIndex newAscentIndex = NormalTable::addRow(parent, columnDataPairs);
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
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, ascent);
	
	NormalTable::updateRow(parent, FORCE_VALID(ascent->ascentID), columnDataPairs);
}


/**
 * Translates the data of an ascent to a list of column-data pairs.
 * 
 * @param columns	The column list specifying the order of the data.
 * @param ascent	The ascent from which to get the data.
 * @return			A list of column-data pairs representing the ascent's data.
 */
const QList<ColumnDataPair> AscentsTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Ascent* ascent) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == titleColumn)				{ data = ascent->title;							}
		else if (column == peakIDColumn)			{ data = ascent->peakID.asQVariant();			}
		else if (column == dateColumn)				{ data = ascent->date;							}
		else if (column == peakOnDayColumn)			{ data = ascent->perDayIndex;					}
		else if (column == timeColumn)				{ data = ascent->time;							}
		else if (column == elevationGainColumn)		{ data = ascent->getElevationGainAsQVariant();	}
		else if (column == hikeKindColumn)			{ data = ascent->hikeKind;						}
		else if (column == traverseColumn)			{ data = ascent->traverse;						}
		else if (column == difficultySystemColumn)	{ data = ascent->difficultySystem;				}
		else if (column == difficultyGradeColumn)	{ data = ascent->difficultyGrade;				}
		else if (column == tripIDColumn)			{ data = ascent->tripID.asQVariant();			}
		else if (column == descriptionColumn)		{ data = ascent->description;					}
		else assert(false);
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns the translated string to be displayed to indicate that no ascent is selected.
 *
 * @return	The translated string representing absence of an ascent.
 */
QString AscentsTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns the translation of "ascent" (singular), not capitalized unless the language requires it.
 *
 * @return	The translation of "ascent" (singular) for use mid-sentence.
 */
QString AscentsTable::getItemNameSingularLowercase() const
{
	return tr("ascent");
}

/**
 * Returns the translation of "ascents" (plural), not capitalized unless the language requires it.
 *
 * @return	The translation of "ascents" (plural) for use mid-sentence.
 */
QString AscentsTable::getItemNamePluralLowercase() const
{
	return tr("ascents");
}
