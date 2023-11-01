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

#include "normal_table.h"

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>



NormalTable::NormalTable(QString name, QString uiName, QString primaryKeyColumnName) :
		Table(name, uiName, false),
		primaryKeyColumn(new Column(primaryKeyColumnName, QString(), ID, false, true, nullptr, this))
{}

NormalTable::~NormalTable()
{}



// BUFFER ACCESS

BufferRowIndex NormalTable::getBufferIndexForPrimaryKey(ValidItemID primaryKey) const
{
	BufferRowIndex index = BufferRowIndex(0);
	for (const QList<QVariant>* const bufferRow : buffer) {
		if (bufferRow->at(0) == primaryKey.get()) return index;
		index++;
	}
	return BufferRowIndex();
}

ValidItemID NormalTable::getPrimaryKeyAt(BufferRowIndex bufferRowIndex) const
{
	return buffer.getCell(bufferRowIndex, primaryKeyColumn->getIndex());
}

QList<QPair<ValidItemID, QVariant>> NormalTable::pairIDWith(const Column* column) const
{
	int primaryKeyColumnIndex = primaryKeyColumn->getIndex();
	int columnIndex = column->getIndex();
	QList<QPair<ValidItemID, QVariant>> pairs = QList<QPair<ValidItemID, QVariant>>();
	for (const QList<QVariant>* const bufferRow : buffer) {
		pairs.append({ValidItemID(bufferRow->at(primaryKeyColumnIndex)), bufferRow->at(columnIndex)});
	}
	return pairs;
}



// MODIFICATIONS (PASSTHROUGH)

BufferRowIndex NormalTable::addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data)
{
	return Table::addRow(parent, columns, data);
}

void NormalTable::updateCell(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data)
{
	return Table::updateCellInNormalTable(parent, primaryKey, column, data);
}

void NormalTable::updateRow(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data)
{
	return Table::updateRowInNormalTable(parent, primaryKey, columns, data);
}



// QABSTRACTIMTEMMODEL IMPLEMENTATION

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
				roleData.setData(buffer.getCell(rowIndex, primaryKeyColumn->getIndex()));
			} else {
				roleData.setData(-1);
			}
			continue;
		}
		const Column* column = getColumnByIndex(columnIndex);
		
		QVariant bufferValue = rowIndex.isInvalid() ? QVariant() : buffer.getCell(rowIndex, columnIndex);
		QVariant result = QVariant();
		
		switch (column->type) {
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
