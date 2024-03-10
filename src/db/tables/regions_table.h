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
 * @file regions_table.h
 * 
 * This file declares the RegionsTable class.
 */

#ifndef REGIONS_TABLE_H
#define REGIONS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/region.h"



/**
 * A class for accessing and manipulating the ascents table in the database.
 */
class RegionsTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The name column. */
	ValueColumn nameColumn;
	/** The range ID column. */
	ForeignKeyColumn rangeIDColumn;
	/** The country ID column. */
	ForeignKeyColumn countryIDColumn;
	
	RegionsTable(Database& db, PrimaryKeyColumn& foreignRangeIDColumn, PrimaryKeyColumn& foreignCountryIDColumn);
	
	BufferRowIndex addRow(QWidget& parent, Region& region);
	void updateRow(QWidget& parent, ValidItemID regionID, const Region& region);
	void updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Region& region);
private:
	const QList<ColumnDataPair> mapDataToColumnDataPairs(const QList<const Column*>& columns, const Region& region) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getCreationConfirmMessage() const;
	virtual QString getEditConfirmMessage(int numEdited) const;
	virtual QString getDeleteConfirmMessage(int numDeleted) const;
	virtual QString getItemCountString(int numItems) const;
};



#endif // REGIONS_TABLE_H
