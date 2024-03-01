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
 @file associative_table.cpp
 * 
 * This file defines the AssociativeTable class.
 */

#include "associative_table.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QModelIndex>



/**
 * Creates a new AssociativeTable and automatically creates and adds its two columns.
 * 
 * @param name					The internal name of the table.
 * @param uiName				The name of the table as it should be displayed in the UI.
 * @param foreignKeyColumn1		The first primary and foreign key column.
 * @param foreignKeyColumn2		The second primary and foreign key column.
 */
AssociativeTable::AssociativeTable(QString name, QString uiName, PrimaryKeyColumn& foreignKeyColumn1, PrimaryKeyColumn& foreignKeyColumn2) :
	Table(name, uiName, true),
	column1(PrimaryForeignKeyColumn(*this, foreignKeyColumn1.name, foreignKeyColumn1.uiName, foreignKeyColumn1)),
	column2(PrimaryForeignKeyColumn(*this, foreignKeyColumn2.name, foreignKeyColumn2.uiName, foreignKeyColumn2))
{
	assert(foreignKeyColumn1.primaryKey && foreignKeyColumn1.type == DataType::ID);
	assert(foreignKeyColumn2.primaryKey && foreignKeyColumn2.type == DataType::ID);
	
	addColumn(column1);
	addColumn(column2);
}

/**
 * Destroys the AssociativeTable.
 */
AssociativeTable::~AssociativeTable()
{}



// COLUMN INFO

/**
 * Returns the first primary and foreign key column of the table.
 * 
 * @return	The first primary and foreign key column of the table.
 */
PrimaryForeignKeyColumn& AssociativeTable::getColumn1()
{
	return column1;
}

/**
 * Returns the second primary and foreign key column of the table.
 * 
 * @return	The second primary and foreign key column of the table.
 */
PrimaryForeignKeyColumn& AssociativeTable::getColumn2()
{
	return column2;
}

/**
 * Given one column, returns the only other column of the table.
 * 
 * @pre The given column is one of the two columns of the table.
 * 
 * @param column	A column of the table.
 * @return			The other column of the table.
 */
const PrimaryForeignKeyColumn& AssociativeTable::getOtherColumn(const PrimaryForeignKeyColumn& column) const
{
	if (&column == &column1) return column2;
	if (&column == &column2) return column1;
	assert(false);
	return column;
}

/**
 * Given a foreign key column, returns the column of this table which references it.
 * 
 * If the given column is not referenced by this table, nullptr is returned.
 * 
 * @param foreignColumn	A key column in a foreign table.
 * @return				The column of this table which references the given column, or nullptr.
 */
const PrimaryForeignKeyColumn* AssociativeTable::getOwnColumnReferencing(const PrimaryKeyColumn& foreignColumn) const
{
	if (column1.foreignColumn == &foreignColumn) return &column1;
	if (column2.foreignColumn == &foreignColumn) return &column2;
	return nullptr;
}

/**
 * Given a foreign key column, returns the table on the other side of the associative relation.
 * 
 * In other words, given a column which is referenced by one of this table's columns, returns the
 * table which is referenced by this table's *other* column.
 * 
 * @pre The given column is referenced by one of this table's columns.
 * 
 * @param foreignColumn	A key column in a foreign table.
 * @return				The table on the other side of the associative relation, or nullptr.
 */
const NormalTable& AssociativeTable::traverseAssociativeRelation(const PrimaryKeyColumn& foreignColumn) const
{
	const PrimaryForeignKeyColumn* matchingOwnColumn = getOwnColumnReferencing(foreignColumn);
	assert(matchingOwnColumn);
	const PrimaryForeignKeyColumn& otherColumn = getOtherColumn(*matchingOwnColumn);
	return (NormalTable&) otherColumn.foreignColumn->table;
}



// BUFFER ACCESS

/**
 * Returns the number of keys on the other side of the given column which are associated with any
 * of the given primary keys from the given column.
 * 
 * @param column		The column in which to search for the given primary keys.
 * @param primaryKeys	The primary keys to search for in the given column.
 * @return				The number of unique keys on the other side of the given column connected to any of the given primary keys.
 */
int AssociativeTable::getNumberOfMatchingOtherPrimaryKeys(const PrimaryForeignKeyColumn& column, const QSet<ValidItemID>& primaryKeys) const
{
	assert(&column == &column1 || &column == &column2);
	const PrimaryForeignKeyColumn& otherColumn = getOtherColumn(column);
	
	QSet<ValidItemID> otherSideKeys = QSet<ValidItemID>();
	for (const QList<QVariant>* const bufferRow : buffer) {
		for (const ValidItemID& primaryKey : primaryKeys) {
			if (Q_UNLIKELY(bufferRow->at(column.getIndex()) == ID_GET(primaryKey))) {
				otherSideKeys.insert(VALID_ITEM_ID(bufferRow->at(otherColumn.getIndex())));
				break;
			}
		}
	}
	return otherSideKeys.size();
}

/**
 * Given a primary key and a column, returns the set of all primary keys in the other column which
 * are associated with it.
 * 
 * In other words, a key is used to search in one column, and for all matching rows, the primary
 * key from the other column is added to the set.
 * 
 * @param column		The column to search in.
 * @param primaryKey	The primary key to search for.
 * @return				The set of all primary keys in the other column which are associated with the given key.
 */
QSet<ValidItemID> AssociativeTable::getMatchingEntries(const PrimaryForeignKeyColumn& column, ValidItemID primaryKey) const
{
	assert(&column == &column1 || &column == &column2);
	const PrimaryForeignKeyColumn& otherColumn = getOtherColumn(column);
	QSet<ValidItemID> filtered = QSet<ValidItemID>();
	for (const QList<QVariant>* const bufferRow : buffer) {
		if (Q_UNLIKELY(bufferRow->at(column.getIndex()) == ID_GET(primaryKey))) {
			filtered.insert(VALID_ITEM_ID(bufferRow->at(otherColumn.getIndex())));
		}
	}
	return filtered;
}



// MODIFICATIONS (PASSTHROUGH)

/**
 * Adds a row to the table from a list of columns and a corresponding list of data.
 * 
 * Delegates to Table::addRow().
 *
 * @param parent			The parent window.
 * @param columnDataPairs	Pairs of columns and corresponding data to add.
 */
void AssociativeTable::addRow(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs)
{
	Table::addRow(parent, columnDataPairs);
}

/**
 * Removes a row from the table, specified by primary keys.
 * 
 * Delegates to Table::removeRow(...).
 *
 * @param parent			The parent window.
 * @param primaryKeyColumns	The primary key columns.
 * @param primaryKeys		The primary keys of the row to remove, in the same order as the columns.
 */
void AssociativeTable::removeRow(QWidget& parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys)
{
	return Table::removeRow(parent, primaryKeyColumns, primaryKeys);
}

/**
 * Removes all rows from the table where the given column has the given value.
 * 
 * Delegates to Table::removeMatchingRows(...).
 *
 * @param parent	The parent window.
 * @param column	The column to check.
 * @param key		The value to check for.
 */
void AssociativeTable::removeMatchingRows(QWidget& parent, const Column& column, ValidItemID primaryKey)
{
	return Table::removeMatchingRows(parent, column, primaryKey);
}

/**
 * Removes all rows from the table where the given column has one of the given values.
 * 
 * Delegates to Table::removeMatchingRows(...).
 *
 * @param parent		The parent window.
 * @param column		The column to check.
 * @param primaryKeys	The values to check for.
 */
void AssociativeTable::removeMatchingRows(QWidget& parent, const Column& column, const QSet<ValidItemID>& primaryKeys)
{
	return Table::removeMatchingRows(parent, column, primaryKeys);
}



// QABSTRACTIMTEMMODEL IMPLEMENTATION

/**
 * For the QAbstractItemModel implementation, fetches the data for the given span of roles and
 * indices (in the form of a QModelRoleDataSpan).
 *
 * The data is not returned, but written back to the given QModelRoleDataSpan.
 */
void AssociativeTable::multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const
{
	assert(getColumnByIndex(index.column()).type == ID);
	
	for (QModelRoleData& roleData : roleDataSpan) {
		int role = roleData.role();
		if (role == Qt::TextAlignmentRole) {
			roleData.setData(Qt::AlignRight);
			continue;
		}
		if (role == Qt::DisplayRole) {
			QVariant result = buffer.getCell(BufferRowIndex(index.row()), index.column());
			roleData.setData(result);
			continue;
		}
	}
}
