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
 * @file participated_table.h
 * 
 * This file declares the ParticipatedTable class.
 */

#ifndef PARTICIPATED_TABLE_H
#define PARTICIPATED_TABLE_H

#include "src/db/associative_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>



/**
 * A class for accessing and manipulating the participated table in the database.
 */
class ParticipatedTable : public AssociativeTable {
public:
	/** The ascent ID column. */
	Column* const ascentIDColumn;
	/** The hiker ID column. */
	Column* const hikerIDColumn;
	
	ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn);
	
	void addRows(QWidget* parent, const Ascent* ascent);
	void updateRows(QWidget* parent, const Ascent* ascent);
private:
	const QList<ColumnDataPair> mapDataToColumnDataPairs(const QList<const Column*>& columns, ValidItemID ascentID, ValidItemID hikerID) const;
};



#endif // PARTICIPATED_TABLE_H
