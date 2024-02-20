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
 @file associative_table.h
 * 
 * This file declares the AssociativeTable class.
 */

#ifndef ASSOCIATIVE_TABLE_H
#define ASSOCIATIVE_TABLE_H

#include "table.h"
#include "normal_table.h"

#include "src/data/item_id.h"


/**
 * A class for accessing and manipulating an associative table in the database.
 * 
 * Only tables associating exactly two other tables are supported.
 */
class AssociativeTable : public Table {
	/** The first primary and foreign key column of the table. */
	PrimaryForeignKeyColumn column1;
	/** The second primary and foreign key column of the table. */
	PrimaryForeignKeyColumn column2;
	
public:
	AssociativeTable(QString name, QString uiName, PrimaryKeyColumn& foreignKeyColumn1, PrimaryKeyColumn& foreignKeyColumn2);
	virtual ~AssociativeTable();
	
	// Column info
	PrimaryForeignKeyColumn& getColumn1();
	PrimaryForeignKeyColumn& getColumn2();
	const PrimaryForeignKeyColumn* getOtherColumn(const PrimaryForeignKeyColumn& column) const;
	const PrimaryForeignKeyColumn* getOwnColumnReferencing(const PrimaryKeyColumn& column) const;
	const NormalTable* traverseAssociativeRelation(const PrimaryKeyColumn& foreignColumn) const;
	
	// Buffer access
	int getNumberOfMatchingRows(const PrimaryForeignKeyColumn& column, ValidItemID primaryKey) const;
	QSet<ValidItemID> getMatchingEntries(const PrimaryForeignKeyColumn& column, ValidItemID primaryKey) const;
	
	// Modifications (passthrough)
	void addRow(QWidget* parent, const QList<ColumnDataPair>& columnDataPairs);
	void removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys);
	void removeMatchingRows(QWidget* parent, const Column& column, ValidItemID primaryKey);
	
	// QAbstractItemModel implementation (completes implementation in Table)
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
};



#endif // ASSOCIATIVE_TABLE_H
