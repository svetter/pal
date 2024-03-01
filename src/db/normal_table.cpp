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
 * @file normal_table.cpp
 * 
 * This file defines the NormalTable class.
 */

#include "normal_table.h"

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>



/**
 * Creates a new NormalTable.
 * 
 * Immediately after construction, all columns should be created and added to the table.
 * 
 * @param name						The internal name of the table.
 * @param uiName					The name of the table as it should be displayed in the UI.
 * @param primaryKeyColumnName		The name of the primary key column.
 * @param primaryKeyColumnUIName	The name of the primary key column as it should be displayed in the UI.
 */
NormalTable::NormalTable(QString name, QString uiName, const QString& primaryKeyColumnName, const QString& primaryKeyColumnUIName) :
	Table(name, uiName, false),
	primaryKeyColumn(PrimaryKeyColumn(*this, primaryKeyColumnName, primaryKeyColumnUIName))
{
	addColumn(primaryKeyColumn);
}

/**
 * Destroys the NormalTable.
 */
NormalTable::~NormalTable()
{}



// BUFFER ACCESS

/**
 * Returns the index of the buffer row which contains the given primary key.
 * 
 * If the primary key is not found, an invalid BufferRowIndex is returned.
 * 
 * @param primaryKey	The primary key to search for.
 * @return				The index of the buffer row which contains the given primary key, or an invalid BufferRowIndex.
 */
BufferRowIndex NormalTable::getBufferIndexForPrimaryKey(ValidItemID primaryKey) const
{
	BufferRowIndex index = BufferRowIndex(0);
	for (const QList<QVariant>* const bufferRow : buffer) {
		if (Q_UNLIKELY(bufferRow->at(0) == ID_GET(primaryKey))) return index;
		index++;
	}
	return BufferRowIndex();
}

/**
 * Returns the primary key of the buffer row at the given index.
 * 
 * @param bufferRowIndex	The index of the buffer row.
 * @return					The primary key of the buffer row at the given index.
 */
ValidItemID NormalTable::getPrimaryKeyAt(BufferRowIndex bufferRowIndex) const
{
	return VALID_ITEM_ID(buffer.getCell(bufferRowIndex, primaryKeyColumn.getIndex()));
}

/**
 * Returns a list of all pairs of primary keys and values of the given column.
 * 
 * @param column	The column to get the values of, aside from the primary keys.
 * @return			A list of pairs of primary keys and values of the given column.
 */
QList<QPair<ValidItemID, QVariant>> NormalTable::pairIDWith(const Column& column) const
{
	int primaryKeyColumnIndex = primaryKeyColumn.getIndex();
	int columnIndex = column.getIndex();
	QList<QPair<ValidItemID, QVariant>> pairs = QList<QPair<ValidItemID, QVariant>>();
	for (const QList<QVariant>* const bufferRow : buffer) {
		pairs.append({VALID_ITEM_ID(bufferRow->at(primaryKeyColumnIndex)), bufferRow->at(columnIndex)});
	}
	return pairs;
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
BufferRowIndex NormalTable::addRow(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs)
{
	return Table::addRow(parent, columnDataPairs);
}

/**
 * Updates a cell in the table, specified by primary key and column.
 *
 * Delegates to Table::updateCellInNormalTable().
 *
 * @param parent		The parent window.
 * @param primaryKey	The primary key of the row to update.
 * @param column		The column to update.
 * @param data			The new data for the cell.
 */
void NormalTable::updateCell(QWidget& parent, const ValidItemID primaryKey, const Column& column, const QVariant& data)
{
	return Table::updateCellInNormalTable(parent, primaryKey, column, data);
}

/**
 * Updates a row in the table, specified by primary key.
 *
 * Delegates to Table::updateRowInNormalTable().
 *
 * @param parent			The parent window.
 * @param primaryKey		The primary key of the row to update.
 * @param columnDataPairs	Pairs of columns and corresponding data to update.
 */
void NormalTable::updateRow(QWidget& parent, const ValidItemID primaryKey, const QList<ColumnDataPair>& columnDataPairs)
{
	return Table::updateRowInNormalTable(parent, primaryKey, columnDataPairs);
}

/**
 * Updates the contents of existing rows in the table, specified by buffer indices.
 * 
 * Delegates to Table::updateRowsInNormalTable().
 * 
 * @param parent			The parent window.
 * @param bufferIndices		The indices of the rows to update in the table buffer.
 * @param columnDataPairs	Pairs of columns and corresponding data to update in all given rows.
 */
void NormalTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& bufferIndices, const QList<ColumnDataPair>& columnDataPairs)
{
	return Table::updateRowsInNormalTable(parent, bufferIndices, columnDataPairs);
}



// QABSTRACTIMTEMMODEL IMPLEMENTATION

/**
 * For the QAbstractItemModel implementation, fetches the formatted data for the given span of
 * roles and indices (in the form of a QModelRoleDataSpan).
 * 
 * The data is not returned, but written back to the given QModelRoleDataSpan.
 * 
 * Cell contents are formatted and aligned according to the column type.
 */
void NormalTable::multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const
{
	for (QModelRoleData& roleData : roleDataSpan) {
		int role = roleData.role();
		
		BufferRowIndex rowIndex;
		if (index.parent().row() == 0) {
			rowIndex = BufferRowIndex(index.row());
		} else {
			rowIndex = BufferRowIndex(index.row() - 1);
		}
		int columnIndex = index.column();
		
		if (role == PrimaryKeyRole) {
			if (rowIndex.isValid()) {
				roleData.setData(buffer.getCell(rowIndex, primaryKeyColumn.getIndex()));
			} else {
				roleData.setData(-1);
			}
			continue;
		}
		const Column& column = getColumnByIndex(columnIndex);
		
		QVariant bufferValue = rowIndex.isInvalid() ? QVariant() : buffer.getCell(rowIndex, columnIndex);
		QVariant result = QVariant();
		
		switch (column.type) {
		case Integer:
		case ID:
		case Enum:
		case DualEnum:
			switch (role) {
			case Qt::DisplayRole:
				if (rowIndex == -1) {
					result = QVariant(getNoneString());
				} else {
					result = bufferValue;
				}
				break;
			case Qt::TextAlignmentRole:
				result = Qt::AlignRight;
				break;
			}
			break;
		case Bit:
			switch (role) {
			case Qt::CheckStateRole:
				result = bufferValue.toBool() ? Qt::Checked : Qt::Unchecked;
				break;
			case Qt::TextAlignmentRole:
				result = Qt::AlignCenter;
				break;
			}
			break;
		case String:
			if (role != Qt::DisplayRole) break;
			if (rowIndex == -1) {
				result = QVariant(getNoneString());
			} else {
				result = bufferValue;
			}
			break;
		case Date:
			if (role == Qt::DisplayRole) {
				result = QVariant(bufferValue.toDate().toString("dd.MM.yyyy"));
			}
			break;
		case Time:
			if (role == Qt::DisplayRole) {
				result = QVariant(bufferValue.toTime().toString("HH:mm"));
			}
			break;
		default:
			assert(false);
		}
		
		roleData.setData(result);
	}
}
