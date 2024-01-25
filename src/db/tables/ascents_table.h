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
 * This file declares the AscentsTable class.
 */

#ifndef ASCENTS_TABLE_H
#define ASCENTS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QWidget>



/**
 * A class for accessing and manipulating the ascents table in the database.
 */
class AscentsTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The ascent title column. */
	ValueColumn* const titleColumn;
	/** The peak ID column. */
	ForeignKeyColumn* const peakIDColumn;
	/** The date column. */
	ValueColumn* const dateColumn;
	/** The peak on day column, specifying that this ascent was the nth ascent on the day of the ascent. */
	ValueColumn* const peakOnDayColumn;
	/** The time column. */
	ValueColumn* const timeColumn;
	/** The elevation gain column. */
	ValueColumn* const elevationGainColumn;
	/** The hike kind column. */
	ValueColumn* const hikeKindColumn;
	/** The traverse column. */
	ValueColumn* const traverseColumn;
	/** The difficulty system column. */
	ValueColumn* const difficultySystemColumn;
	/** The difficulty grade column. */
	ValueColumn* const difficultyGradeColumn;
	/** The trip ID column. */
	ForeignKeyColumn* const tripIDColumn;
	/** The ascent description column. */
	ValueColumn* const descriptionColumn;
	
	AscentsTable(PrimaryKeyColumn* foreignPeakIDColumn, PrimaryKeyColumn* foreignTripIDColumn);
	
	BufferRowIndex addRow(QWidget* parent, Ascent* ascent);
	void updateRow(QWidget* parent, const Ascent* ascent);
private:
	const QList<ColumnDataPair> mapDataToColumnDataPairs(const QList<const Column*>& columns, const Ascent* ascent) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // ASCENTS_TABLE_H
