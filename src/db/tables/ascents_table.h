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
	Column* const titleColumn;
	/** The peak ID column. */
	Column* const peakIDColumn;
	/** The date column. */
	Column* const dateColumn;
	/** The peak on day column, specifying that this ascent was the nth ascent on the day of the ascent. */
	Column* const peakOnDayColumn;
	/** The time column. */
	Column* const timeColumn;
	/** The elevation gain column. */
	Column* const elevationGainColumn;
	/** The hike kind column. */
	Column* const hikeKindColumn;
	/** The traverse column. */
	Column* const traverseColumn;
	/** The difficulty system column. */
	Column* const difficultySystemColumn;
	/** The difficulty grade column. */
	Column* const difficultyGradeColumn;
	/** The trip ID column. */
	Column* const tripIDColumn;
	/** The ascent description column. */
	Column* const descriptionColumn;
	
	AscentsTable(Column* foreignPeakIDColumn, Column* foreignTripIDColumn);
	
	BufferRowIndex addRow(QWidget* parent, Ascent* ascent);
	void updateRow(QWidget* parent, const Ascent* ascent);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Ascent* ascent) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // ASCENTS_TABLE_H
