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
 * @file ranges_table.h
 * 
 * This file declares the RangesTable class.
 */

#ifndef RANGES_TABLE_H
#define RANGES_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/range.h"

#include <QWidget>



/**
 * A class for accessing and manipulating the ascents table in the database.
 */
class RangesTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The name column. */
	ValueColumn nameColumn;
	/** The continent column. */
	ValueColumn continentColumn;
	
	RangesTable(Database& db);
	
	BufferRowIndex addRow(QWidget& parent, Range& range);
	void updateRow(QWidget& parent, ValidItemID rangeID, const Range& range);
	void updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Range& range);
private:
	const QList<ColumnDataPair> mapDataToColumnDataPairs(const QList<const Column*>& columns, const Range& range) const;
	
public:
	virtual QString getItemNameSingular() const;
	virtual QString getNoneString() const;
	virtual QString getCreationConfirmMessage() const;
	virtual QString getEditConfirmMessage(int numEdited) const;
	virtual QString getDeleteConfirmMessage(int numDeleted) const;
	virtual QString getNewCustomColumnString() const;
	virtual QString getNewFilterString() const;
	virtual QString getItemCountString(int numItems) const;
};



#endif // RANGES_TABLE_H
