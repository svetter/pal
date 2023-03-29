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

#include "table.h"

#include "db_error.h"
#include "src/comp_tables/composite_table.h"

#include <QSqlQuery>



Table::Table(QString name, QString uiName, bool isAssociative) :
		rowChangeListener(nullptr),
		name(name),
		uiName(uiName),
		isAssociative(isAssociative),
		buffer(QList<QList<QVariant>*>())
{}

Table::~Table()
{
	qDeleteAll(columns);
	qDeleteAll(buffer);
}



void Table::addColumn(const Column* column)
{
	columns.append(column);
}



// COLUMN INFO

int Table::getNumberOfColumns() const
{
	return columns.size();
}

int Table::getNumberOfPrimaryKeyColumns() const
{
	int numberOfPrimaryKeyColumns = 0;
	for (const Column* column : columns) {
		if (column->primaryKey) numberOfPrimaryKeyColumns++;
	}
	return numberOfPrimaryKeyColumns;
}

int Table::getNumberOfNonPrimaryKeyColumns() const
{
	return getNumberOfColumns() - getNumberOfPrimaryKeyColumns();
}

QList<const Column*> Table::getColumnList() const
{
	return QList<const Column*>(columns);
}

QList<const Column*> Table::getPrimaryKeyColumnList() const
{
	QList<const Column*> primaryKeyColumns = QList<const Column*>();
	for (const Column* column : columns) {
		if (column->primaryKey) primaryKeyColumns.append(column);
	}
	return primaryKeyColumns;
}

QList<const Column*> Table::getNonPrimaryKeyColumnList() const
{
	QList<const Column*> primaryKeyColumns = QList<const Column*>();
	for (const Column* column : columns) {
		if (!column->primaryKey) primaryKeyColumns.append(column);
	}
	return primaryKeyColumns;
}

QString Table::getColumnListString() const
{
	return getColumnListStringOf(getColumnList());
}

QString Table::getPrimaryKeyColumnListString() const
{
	return getColumnListStringOf(getPrimaryKeyColumnList());
}

int Table::getColumnIndex(const Column* column) const
{
	return columns.indexOf(column);
}

const Column* Table::getColumnByIndex(int index) const
{
	assert(index >= 0 && index < columns.size());
	return columns.at(index);
}



// BUFFER ACCESS

void Table::initBuffer(QWidget* parent, bool expectEmpty)
{
	QList<QList<QVariant>*> newContents = getAllEntriesFromSql(parent, expectEmpty);
	beginInsertRows(getNormalRootModelIndex(), 0, newContents.size() - 1);
	buffer.clear();
	for (QList<QVariant>* newRow : newContents) {
		buffer.append(newRow);
	}
	endInsertRows();
}

void Table::resetBuffer()
{
	beginRemoveRows(getNormalRootModelIndex(), 0, buffer.size() - 1);
	qDeleteAll(buffer);
	buffer.clear();
	endRemoveRows();
}

int Table::getNumberOfRows() const
{
	return buffer.size();
}

const QList<QVariant>* Table::getBufferRow(int rowIndex) const
{
	return buffer.at(rowIndex);
}

QList<int> Table::getMatchingBufferRowIndices(const Column* column, const QVariant& content) const
{
	assert(getColumnList().contains(column));
	
	QList<int> result = QList<int>();
	for (int rowIndex = 0; rowIndex < buffer.size(); rowIndex++) {
		if (column->getValueAt(rowIndex) == content) {
			result.append(rowIndex);
		}
	}
	return result;
}

int Table::getMatchingBufferRowIndex(const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys) const
{
	int numPrimaryKeys = getPrimaryKeyColumnList().size();
	assert(primaryKeyColumns.size() == numPrimaryKeys);
	assert(primaryKeys.size() == numPrimaryKeys);
	
	for (int rowIndex = 0; rowIndex < buffer.size(); rowIndex++) {
		bool match = true;
		for (int i = 0; i < numPrimaryKeys; i++) {
			if (primaryKeyColumns.at(i)->getValueAt(rowIndex) != primaryKeys.at(i).get()) {
				match = false;
				break;
			}
		}
		if (match) return rowIndex;
	}
	assert(false);
	return -1;
}


void Table::printBuffer() const
{
	qDebug() << "Printing buffer of" << name;
	QString header = "";
	for (const Column* column : getColumnList()) {
		header.append(column->name + "  ");
	}
	qDebug() << header;
	for (QList<QVariant>* bufferRow : buffer) {
		QString rowString = "";
		for (int columnIndex = 0; columnIndex < getNumberOfColumns(); columnIndex++) {
			rowString.append(bufferRow->at(columnIndex).toString()).append("        ");
		}
		qDebug() << rowString;
	}
}



// CHANGE PROPAGATION

void Table::setRowChangeListener(CompositeTable* compositeTable)
{
	rowChangeListener = compositeTable;
}

void Table::notifyAllColumns()
{
	// Collect change listeners and notify them
	QSet<const CompositeColumn*> changeListeners = QSet<const CompositeColumn*>();
	for (const Column* column : columns) {
		changeListeners.unite(column->getChangeListeners());
	}
	for (const CompositeColumn* changeListener : changeListeners) {
		changeListener->announceChangedData();
	}
}



// MODIFICATIONS

int Table::addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data)
{	
	assert(columns.size() == data.size());
	
	// Announce row insertion
	int newItemBufferRowIndex = buffer.size();
	beginInsertRows(getNormalRootModelIndex(), newItemBufferRowIndex, newItemBufferRowIndex);
	beginInsertRows(getNullableRootModelIndex(), newItemBufferRowIndex, newItemBufferRowIndex);
	
	// Add data to SQL database
	int newRowID = addRowToSql(parent, columns, data);
	
	// Update buffer
	QList<QVariant>* newBufferRow = new QList<QVariant>(data);
	if (!isAssociative) {
		newBufferRow->insert(0, newRowID);
	}
	buffer.append(newBufferRow);
	
	// Announce end of row insertion
	endInsertRows();
	if (rowChangeListener) rowChangeListener->bufferRowJustInserted(newItemBufferRowIndex);
	
	// Row was added, all columns affected => Notify all column-attached change listeners
	notifyAllColumns();
	
	return newItemBufferRowIndex;
}

void Table::updateCellInNormalTable(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data)
{
	assert(!isAssociative);
	QList<const Column*> primaryKeyColumns = getPrimaryKeyColumnList();
	assert(primaryKeyColumns.size() == 1);
	assert(column->table == this);
	
	// Update cell in SQL database
	updateCellInSql(parent, primaryKey, column, data);
	
	// Update buffer
	int bufferRowIndex = getMatchingBufferRowIndex(primaryKeyColumns, { primaryKey });
	buffer.at(bufferRowIndex)->replace(column->getIndex(), data);
	
	// Announce changed data
	QModelIndex updateIndexNormal	= index(bufferRowIndex, column->getIndex(), getNormalRootModelIndex());
	QModelIndex updateIndexNullable	= index(bufferRowIndex, column->getIndex(), getNullableRootModelIndex());
	const QList<int> updatedDatumRoles = { column->type == Bit ? Qt::CheckStateRole : Qt::DisplayRole };
	Q_EMIT dataChanged(updateIndexNormal, updateIndexNormal, updatedDatumRoles);
	Q_EMIT dataChanged(updateIndexNullable, updateIndexNullable, updatedDatumRoles);
	// Collect column's change listeners and notify them
	QSet<const CompositeColumn*> changeListeners = column->getChangeListeners();
	for (const CompositeColumn* changeListener : changeListeners) {
		changeListener->announceChangedData();
	}
}

void Table::updateRowInNormalTable(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data)
{
	assert(!isAssociative);
	QList<const Column*> primaryKeyColumns = getPrimaryKeyColumnList();
	assert(primaryKeyColumns.size() == 1);
	
	// Update cell in SQL database
	updateRowInSql(parent, primaryKey, columns, data);
	
	// Update buffer
	int bufferRowIndex = getMatchingBufferRowIndex(primaryKeyColumns, { primaryKey });
	for (int i = 0; i < columns.size(); i++) {
		buffer.at(bufferRowIndex)->replace(columns.at(i)->getIndex(), data.at(i));
	}
	
	// Announce changed data
	QModelIndex updateIndexLeft = index(bufferRowIndex, 0, getNormalRootModelIndex());
	QModelIndex updateIndexRight = index(bufferRowIndex, getNumberOfColumns(), getNormalRootModelIndex());
	const QList<int> updatedDatumRoles = { Qt::CheckStateRole, Qt::DisplayRole };
	Q_EMIT dataChanged(updateIndexLeft, updateIndexRight, updatedDatumRoles);
	// Whole row was updated, all columns affected => Notify all column-attached change listeners
	notifyAllColumns();
}

void Table::removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys)
{
	int numPrimaryKeys = getNumberOfPrimaryKeyColumns();
	assert(primaryKeyColumns.size() == numPrimaryKeys);
	assert(primaryKeys.size() == numPrimaryKeys);
	
	int bufferRowIndex = getMatchingBufferRowIndex(primaryKeyColumns, primaryKeys);
	assert(bufferRowIndex >= 0);
	
	// Announce row removal
	beginRemoveRows(getNormalRootModelIndex(), bufferRowIndex, bufferRowIndex);
	if (rowChangeListener) rowChangeListener->bufferRowAboutToBeRemoved(bufferRowIndex);
	
	// Remove row from SQL database
	removeRowFromSql(parent, primaryKeyColumns, primaryKeys);
	
	// Update buffer
	const QList<QVariant>* rowToRemove = getBufferRow(bufferRowIndex);
	buffer.remove(bufferRowIndex);
	delete rowToRemove;
	
	// Announce end of row removal
	endRemoveRows();
	// Row was removed, all columns affected => Notify all column-attached change listeners
	notifyAllColumns();
}

void Table::removeMatchingRows(QWidget* parent, const Column* column, ValidItemID key)
{
	assert(getColumnList().contains(column));
	assert(column->isKey());
	
	// Remove rows from SQL database
	removeMatchingRowsFromSql(parent, column, key);
	
	// Update buffer
	QList<int> bufferRowIndices = getMatchingBufferRowIndices(column, key.asQVariant());
	if (bufferRowIndices.isEmpty()) return;
	
	auto iter = bufferRowIndices.constEnd();
	while (iter-- != bufferRowIndices.constBegin()) {
		// Announce row removal
		int bufferRowIndex = *iter;
		beginRemoveRows(getNormalRootModelIndex(), bufferRowIndex, bufferRowIndex);
		if (rowChangeListener) rowChangeListener->bufferRowAboutToBeRemoved(bufferRowIndex);
		
		const QList<QVariant>* rowToRemove = getBufferRow(bufferRowIndex);
		buffer.remove(bufferRowIndex);
		delete rowToRemove;
		
		// Announce end of row removal
		endRemoveRows();
	}
	
	// Rows were removed, all columns affected => Notify all column-attached change listeners
	notifyAllColumns();
}



// SQL

void Table::createTableInSql(QWidget* parent)
{
	QString columnFormatsString = "";
	for (int i = 0; i < columns.size(); i++) {
		if (i > 0) columnFormatsString.append(",\n");
		columnFormatsString.append("\t" + columns.at(i)->getSqlSpecificationString());
	}
	
	QString associativeString;
	if (isAssociative) {
		associativeString = "CONSTRAINT " + name.toLower() + "PK PRIMARY KEY (" + getPrimaryKeyColumnListString() + ")";
	}
	
	QString queryString = QString(
			"CREATE TABLE " + name + "(" +
			"\n" + columnFormatsString +
			(isAssociative ? (",\n\t" + associativeString) : "") +
			"\n)"
	);
	qDebug() << queryString;
	QSqlQuery query = QSqlQuery();
	
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
}

QList<QList<QVariant>*> Table::getAllEntriesFromSql(QWidget* parent, bool expectEmpty) const
{
	QString queryString = QString(
			"SELECT " + getColumnListString() +
			"\nFROM " + name
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	QList<QList<QVariant>*> result = QList<QList<QVariant>*>();
	
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
	
	QList<const Column*> columns = getColumnList();
	
	int rowIndex = 0;
	while (query.next()) {
		result.append(new QList<QVariant>());
		int columnIndex = 0;
		for (const Column* column : columns) {
			QVariant value = query.value(columnIndex);
			assert(column->nullable || !value.isNull());
			if (value.isNull()) value = QVariant();
			result.at(rowIndex)->append(value);
			columnIndex++;
		}
		rowIndex++;
	}
	
	if (!expectEmpty && result.empty()) {
		qDebug() << "Couldn't read record from SQL query, or it returned nothing:";
		qDebug() << queryString;
	}
	return result;
}

int Table::addRowToSql(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data)
{
	QString questionMarks = "";
	for (int i = 0; i < columns.size(); i++) {
		questionMarks = questionMarks + ((i == 0) ? "?" : ", ?");
	}
	QString queryString = QString(
			"INSERT INTO " + name + "(" + getColumnListStringOf(columns) + ")" +
			"\nVALUES(" + questionMarks + ")"
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString))
		displayError(parent, query.lastError(), queryString);
	for (auto iter = data.constBegin(); iter != data.constEnd(); iter++) {
		query.addBindValue(*iter);
	}
	
	if (!query.exec())
		displayError(parent, query.lastError(), queryString);
	
	int newRowID = query.lastInsertId().toInt();
	assert(newRowID > 0);
	return newRowID;
}

void Table::updateCellInSql(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data)
{
	auto primaryKeyColumns = getPrimaryKeyColumnList();
	const Column* primaryKeyColumn = primaryKeyColumns.first();
	
	QString queryString = QString(
			"UPDATE " + name + 
			"\nSET " + column->name + " = ?" +
			"\nWHERE " + primaryKeyColumn->name + " = " + QString::number(primaryKey.get())
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString))
		displayError(parent, query.lastError(), queryString);
	query.addBindValue(data);
	
	if (!query.exec())
		displayError(parent, query.lastError(), queryString);
}

void Table::updateRowInSql(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data)
{
	auto primaryKeyColumns = getPrimaryKeyColumnList();
	const Column* primaryKeyColumn = primaryKeyColumns.first();
	
	QString setString = "";
	for (int i = 0; i < columns.size(); i++) {
		if (i > 0) setString.append(", ");
		setString.append(columns.at(i)->name).append(" = ?");
	}
	QString queryString = QString(
			"UPDATE " + name + 
			"\nSET " + setString +
			"\nWHERE " + primaryKeyColumn->name + " = " + QString::number(primaryKey.get())
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString))
		displayError(parent, query.lastError(), queryString);
	for (int i = 0; i < data.size(); i++) {
		query.addBindValue(data.at(i));
	}
	
	if (!query.exec())
		displayError(parent, query.lastError(), queryString);
}

void Table::removeRowFromSql(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys)
{
	QString condition = "";
	for (int i = 0; i < primaryKeys.size(); i++) {
		if (i > 0) condition.append(" AND ");
		const Column* column = primaryKeyColumns.at(i);
		assert(column->table == this && column->isPrimaryKey());
		
		condition.append(column->name + " = " + QString::number(primaryKeys.at(i).get()));
	}
	QString queryString = QString(
			"DELETE FROM " + name +
			"\nWHERE " + condition
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
}

void Table::removeMatchingRowsFromSql(QWidget* parent, const Column* column, ValidItemID key)
{
	assert(getColumnList().contains(column));
	
	QString queryString = QString(
			"DELETE FROM " + name +
			"\nWHERE " + column->name + " = " + QString::number(key.get())
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
}



// QABSTRACTMODEL IMPLEMENTATION

QModelIndex Table::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent)) {
		qDebug() << QString("NormalTable::index() called with unrecognized location: row %1, column %2, parent").arg(row).arg(column) << parent;
		return QModelIndex();
	}
	if (!parent.isValid()) {
		return createIndex(row, column, -(row + 1));
	}
	return createIndex(row, column, parent.row());
}

QModelIndex Table::parent(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}
	long long ptr = (long long) index.internalPointer();
	if (ptr < 0) {
		return QModelIndex();
	} else {
		return createIndex(ptr, 0, -(ptr + 1));
	}
}

int Table::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid()) return 2;
	int numberActualRows = buffer.size();
	if (parent.row() == 0) {
		return numberActualRows;
	} else {
		return numberActualRows + 1;
	}
}

int Table::columnCount(const QModelIndex& parent) const
{
	if (!parent.isValid()) return 1;
	return getNumberOfColumns();
}

QVariant Table::data(const QModelIndex& index, int role) const
{
	QModelRoleData roleData(role);
	multiData(index, roleData);
	return roleData.data();
}

QVariant Table::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Orientation::Vertical) {
		switch (role) {
		case Qt::TextAlignmentRole:	return Qt::AlignRight;
		case Qt::DisplayRole:		return section + 1;
		}
		return QVariant();
	}
	
	if (role != Qt::DisplayRole) return QVariant();
	if (orientation != Qt::Orientation::Horizontal) return QVariant(section + 1);
	QString result = getColumnByIndex(section)->uiName;
	if (result.isEmpty()) result = getColumnByIndex(section)->name;
	return result;
}


const int Table::PrimaryKeyRole = -1;

QModelIndex Table::getNormalRootModelIndex() const
{
	return index(0, 0);
}

QModelIndex Table::getNullableRootModelIndex() const
{
	return index(1, 0);
}
