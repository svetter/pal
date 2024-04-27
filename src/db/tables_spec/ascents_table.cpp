/*
 * Copyright 2023-2024 Simon Vetter
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
#include "src/db/tables_spec/peaks_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new AscentsTable.
 * 
 * @param db					The database to which the table belongs.
 * @param foreignPeakIDColumn	The primary key column of the PeaksTable.
 * @param foreignTripIDColumn	The primary key column of the TripsTable.
 */
AscentsTable::AscentsTable(Database& db, PrimaryKeyColumn& foreignPeakIDColumn,  PrimaryKeyColumn& foreignTripIDColumn) :
	NormalTable(db, QString("Ascents"), tr("Ascents"), "ascentID", tr("Ascent ID")),
	//													name				uiName				type		nullable	foreignColumn/enumNames/enumNameLists
	titleColumn				(ValueColumn		(*this,	"title",			tr("Title"),		String,		true)),
	peakIDColumn			(ForeignKeyColumn	(*this,	"peakID",			tr("Peak ID"),					true,		foreignPeakIDColumn)),
	dateColumn				(ValueColumn		(*this,	"date",				tr("Date"),			Date,		true)),
	peakOnDayColumn			(ValueColumn		(*this,	"peakOnDay",		tr("Peak/day"),		Integer,	false)),
	timeColumn				(ValueColumn		(*this,	"time",				tr("Local time"),	Time,		true)),
	elevationGainColumn		(ValueColumn		(*this,	"elevationGain",	tr("Elev. gain"),	Integer,	true)),
	hikeKindColumn			(ValueColumn		(*this,	"hikeKind",			tr("Kind of hike"),	Enum,		false,		&EnumNames::hikeKindNames)),
	traverseColumn			(ValueColumn		(*this,	"traverse",			tr("Traverse"),		Bit,		false)),
	difficultySystemColumn	(ValueColumn		(*this,	"difficultySystem",	tr("Diff. system"),	DualEnum,	false,		nullptr,	&EnumNames::difficultyNames)),
	difficultyGradeColumn	(ValueColumn		(*this,	"difficultyGrade",	tr("Diff. grade"),	DualEnum,	false,		nullptr,	&EnumNames::difficultyNames)),
	tripIDColumn			(ForeignKeyColumn	(*this,	"tripID",			tr("Trip ID"),					true,		foreignTripIDColumn)),
	descriptionColumn		(ValueColumn		(*this,	"description",		tr("Description"),	String,		true))
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



/**
 * Adds a new ascent to the table.
 * 
 * @param parent	The parent widget.
 * @param ascent	The ascent to add.
 * @return			The index of the new ascent in the table buffer.
 */
BufferRowIndex AscentsTable::addRow(QWidget& parent, Ascent& ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, ascent);
	
	BufferRowIndex newAscentIndex = NormalTable::addRow(parent, columnDataPairs);
	ascent.ascentID = getPrimaryKeyAt(newAscentIndex);
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
void AscentsTable::updateRow(QWidget& parent, const Ascent& ascent)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, ascent);
	
	NormalTable::updateRow(parent, FORCE_VALID(ascent.ascentID), columnDataPairs);
}

/**
 * Updates the contents of existing ascents in the table.
 * 
 * @param parent		The parent widget.
 * @param rowIndices	The indices of the ascents to update in the table buffer.
 * @param columns		The columns to update. Can not contain the primary key column.
 * @param ascent		The new data for the ascents. The ascentID will be ignored.
 */
void AscentsTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Ascent& ascent)
{
	assert(!columns.contains(&primaryKeyColumn));
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, ascent);
	
	NormalTable::updateRows(parent, rowIndices, columnDataPairs);
}


/**
 * Translates the data of an ascent to a list of column-data pairs.
 * 
 * @param columns	The column list specifying the order of the data.
 * @param ascent	The ascent from which to get the data.
 * @return			A list of column-data pairs representing the ascent's data.
 */
const QList<ColumnDataPair> AscentsTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Ascent& ascent) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &titleColumn)			{ data = ascent.title;							}
		else if (column == &peakIDColumn)			{ data = ascent.peakID.asQVariant();			}
		else if (column == &dateColumn)				{ data = ascent.date;							}
		else if (column == &peakOnDayColumn)		{ data = ascent.perDayIndex;					}
		else if (column == &timeColumn)				{ data = ascent.time;							}
		else if (column == &elevationGainColumn)	{ data = ascent.getElevationGainAsQVariant();	}
		else if (column == &hikeKindColumn)			{ data = ascent.hikeKind;						}
		else if (column == &traverseColumn)			{ data = ascent.traverse;						}
		else if (column == &difficultySystemColumn)	{ data = ascent.difficultySystem;				}
		else if (column == &difficultyGradeColumn)	{ data = ascent.difficultyGrade;				}
		else if (column == &tripIDColumn)			{ data = ascent.tripID.asQVariant();			}
		else if (column == &descriptionColumn)		{ data = ascent.description;					}
		else assert(false);
		
		if (column->type == String && data.toString().isEmpty()) data = QVariant();
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns a string representation of the ascent at the given buffer row index.
 *
 * @param bufferRow	The buffer row index of the ascent to represent.
 * @return			A UI-appropriate string representation of the ascent.
 */
QString AscentsTable::getIdentityRepresentationAt(const BufferRowIndex& bufferRow) const
{
	const QVariant dateRaw = dateColumn.getValueAt(bufferRow);
	QString dateString = QString();
	if (dateRaw.isValid() && dateRaw.canConvert<QDate>()) {
		const QDate date = dateRaw.toDate();
		dateString = date.toString(Qt::ISODate);;
	}
	
	const ItemID peakID = peakIDColumn.getValueAt(bufferRow);
	QString peakString = QString();
	if (peakID.isValid()) {
		const PeaksTable& peaksTable = (PeaksTable&) peakIDColumn.foreignColumn->table;
		peakString = peaksTable.nameColumn.getValueFor(FORCE_VALID(peakID)).toString();
	}
	
	const QString separator = (!dateString.isEmpty() && !peakString.isEmpty()) ? ": " : "";
	return dateString + separator + peakString;
}

/**
 * Returns a set of all columns used for identity representation of ascents.
 *
 * @return	A set of all columns used for identity representation.
 */
QSet<const Column*> AscentsTable::getIdentityRepresentationColumns() const
{
	const PeaksTable& peaksTable = (PeaksTable&) peakIDColumn.foreignColumn->table;
	
	return { &dateColumn, &peakIDColumn, &peaksTable.primaryKeyColumn, &peaksTable.nameColumn };
}



/**
 * Returns the translation of "Ascent" (singular).
 * 
 * @return	The translation of "Ascent" (singular).
 */
QString AscentsTable::getItemNameSingular() const
{
	return tr("Ascent");
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
 * Returns a translated message confirming that a new ascent has been created.
 * 
 * @return	The translated message confirming creation of a new ascent.
 */
QString AscentsTable::getCreationConfirmMessage() const
{
	return tr("Saved new ascent.");
}

/**
 * Returns a translated message confirming that a number of ascents have been edited.
 *
 * @param numEdited	The number of ascents that have been edited.
 * @return			The translated message confirming the editing of the ascents.
 */
QString AscentsTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln ascent(s).", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of ascents have been deleted.
 *
 * @param numDeleted	The number of ascents that have been deleted.
 * @return				The translated message confirming the deletion of the ascents.
 */
QString AscentsTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln ascent(s).", "", numDeleted);
}

/**
 * Returns a translated title for a ascent custom column wizard.
 * 
 * @return	The translated title for a ascent custom column wizard.
 */
QString AscentsTable::getNewCustomColumnString() const
{
	return tr("New custom column for ascents");
}

/**
 * Returns a translated title for an ascent filter wizard.
 * 
 * @return	The translated title for an ascent filter wizard.
 */
QString AscentsTable::getNewFilterString() const
{
	return tr("New ascent filter");
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n ascent(s)".
 * 
 * @param numItems	The number of ascents to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString AscentsTable::getItemCountString(int numItems) const
{
	return tr("%Ln ascent(s)", "", numItems);
}
