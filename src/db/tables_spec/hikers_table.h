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
 * @file hikers_table.h
 * 
 * This file declares the HikersTable class.
 */

#ifndef HIKERS_TABLE_H
#define HIKERS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/hiker.h"

#include <QWidget>



/**
 * A class for accessing and manipulating the ascents table in the database.
 */
class HikersTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The name column. */
	ValueColumn nameColumn;
	
	HikersTable(Database& db);
	
	BufferRowIndex addRow(QWidget& parent, Hiker& hiker);
	void updateRow(QWidget& parent, ValidItemID hikerID, const Hiker& hiker);
	void updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Hiker& hiker);
private:
	const QList<ColumnDataPair> mapDataToColumnDataPairs(const QList<const Column*>& columns, const Hiker& hiker) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getCreationConfirmMessage() const;
	virtual QString getEditConfirmMessage(int numEdited) const;
	virtual QString getDeleteConfirmMessage(int numDeleted) const;
	virtual QString getItemCountString(int numItems) const;
};



#endif // HIKERS_TABLE_H
