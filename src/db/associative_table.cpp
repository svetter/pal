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
 * @param foreignKeyColumn1		The first primary and foreign key column.
 * @param foreignKeyColumn2		The second primary and foreign key column.
 */
AssociativeTable::AssociativeTable(QString name, Column* foreignKeyColumn1, Column* foreignKeyColumn2) :
		Table(name, QString(), true),
		column1(new Column(foreignKeyColumn1->name, foreignKeyColumn1->uiName, DataType::ID, false, true, foreignKeyColumn1, this)),
		column2(new Column(foreignKeyColumn2->name, foreignKeyColumn2->uiName, DataType::ID, false, true, foreignKeyColumn2, this))
{
	assert(foreignKeyColumn1->primaryKey && foreignKeyColumn1->type == DataType::ID);
	assert(foreignKeyColumn2->primaryKey && foreignKeyColumn2->type == DataType::ID);
	
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
Column* AssociativeTable::getColumn1() const
{
	return column1;
}

/**
 * Returns the second primary and foreign key column of the table.
 * 
 * @return	The second primary and foreign key column of the table.
 */
Column* AssociativeTable::getColumn2() const
{
	return column2;
}

/**
 * Given one column, returns the only other column of the table.
 * 
 * @param column	A column of the table.
 * @return			The other column of the table.
 */
const Column* AssociativeTable::getOtherColumn(const Column* column) const
{
	if (column == column1) return column2;
	if (column == column2) return column1;
	return nullptr;
}

/**
 * Given a foreign key column, returns the column of this table which references it.
 * 
 * If the given column is not referenced by this table, nullptr is returned.
 * 
 * @param foreignColumn	A key column in a foreign table.
 * @return				The column of this table which references the given column, or nullptr.
 */
const Column* AssociativeTable::getOwnColumnReferencing(const Column* foreignColumn) const
{
	if (column1->foreignKey == foreignColumn)	return column1;
	if (column2->foreignKey == foreignColumn)	return column2;
	return nullptr;
}

/**
 * Given a foreign key column, returns the table on the other side of the associative relation.
 * 
 * In other words, given a column which is referenced by one of this table's columns, returns the
 * table which is referenced by this table's *other* column.
 * 
 * If the given column is not referenced by this table, nullptr is returned.
 * 
 * @param foreignColumn	A key column in a foreign table.
 * @return				The table on the other side of the associative relation, or nullptr.
 */
const NormalTable* AssociativeTable::traverseAssociativeRelation(const Column* foreignColumn) const
{
	const Column* matchingOwnColumn = getOwnColumnReferencing(foreignColumn);
	if (!matchingOwnColumn) return nullptr;
	
	const Column* otherColumn = getOtherColumn(matchingOwnColumn);
	return (NormalTable*) otherColumn->foreignKey->table;
}



// BUFFER ACCESS

/**
 * Returns the number of rows in the buffer which match the given primary key in the given column.
 * 
 * @param column		The column to search in.
 * @param primaryKey	The primary key to search for.
 * @return				The number of rows in the buffer which match the given primary key in the given column.
 */
int AssociativeTable::getNumberOfMatchingRows(const Column* column, ValidItemID primaryKey) const
{
	assert(column == column1 || column == column2);
	int numberOfMatches = 0;
	for (const QList<QVariant>* const bufferRow : buffer) {
		if (bufferRow->at(column->getIndex()) == primaryKey.get()) {
			numberOfMatches++;
		}
	}
	return numberOfMatches;
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
QSet<ValidItemID> AssociativeTable::getMatchingEntries(const Column* column, ValidItemID primaryKey) const
{
	assert(column == column1 || column == column2);
	const Column* otherColumn = getOtherColumn(column);
	QSet<ValidItemID> filtered = QSet<ValidItemID>();
	for (const QList<QVariant>* const bufferRow : buffer) {
		if (bufferRow->at(column->getIndex()) == primaryKey.get()) {
			filtered.insert(bufferRow->at(otherColumn->getIndex()));
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
void AssociativeTable::addRow(QWidget* parent, const QList<ColumnDataPair>& columnDataPairs)
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
void AssociativeTable::removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys)
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
void AssociativeTable::removeMatchingRows(QWidget* parent, const Column* column, ValidItemID primaryKey)
{
	return Table::removeMatchingRows(parent, column, primaryKey);
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
	assert(getColumnByIndex(index.column())->type == ID);
	
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
