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
 * @file table.cpp
 * 
 * This file defines the Table class.
 */

#include "table.h"

#include "db_error.h"
#include "database.h"

#include <QSqlQuery>
#include <QDateTime>

using std::unique_ptr, std::shared_ptr;



/**
 * Creates a new Table.
 * 
 * @param db			The database to which the table belongs.
 * @param name			The name of the table in the SQL database.
 * @param uiName		The name of the table as it should be displayed in the UI.
 * @param isAssociative	Whether the table is associative.
 */
Table::Table(Database& db, QString name, QString uiName, bool isAssociative) :
	db(db),
	name(name),
	uiName(uiName),
	isAssociative(isAssociative),
	buffer(TableBuffer())
{}

/**
 * Destroys the Table.
 */
Table::~Table()
{}



/**
 * Adds a column to the table during initialization.
 * 
 * @param newColumn	The column to add. The table takes ownership of the column.
 */
void Table::addColumn(const Column& newColumn)
{
	for (const Column* const column : columns) {
		assert(column->name != newColumn.name);
	}
	
	columns.append(&newColumn);
}



// COLUMN INFO

/**
 * Returns the number of columns in the table.
 * 
 * @return	The number of columns in the table.
 */
int Table::getNumberOfColumns() const
{
	return columns.size();
}

/**
 * Returns the number of primary key columns in the table.
 * 
 * @return	The number of primary key columns in the table.
 */
int Table::getNumberOfPrimaryKeyColumns() const
{
	int numberOfPrimaryKeyColumns = 0;
	for (const Column* column : columns) {
		if (column->primaryKey) numberOfPrimaryKeyColumns++;
	}
	return numberOfPrimaryKeyColumns;
}

/**
 * Returns a list of all columns in the table.
 * 
 * @return	A list of all columns in the table.
 */
QList<const Column*> Table::getColumnList() const
{
	return QList<const Column*>(columns);
}

/**
 * Returns a list of all primary key columns in the table.
 * 
 * @return	A list of all primary key columns in the table.
 */
QList<const Column*> Table::getPrimaryKeyColumnList() const
{
	QList<const Column*> primaryKeyColumns = QList<const Column*>();
	for (const Column* column : columns) {
		if (column->primaryKey) primaryKeyColumns.append(column);
	}
	return primaryKeyColumns;
}

/**
 * Returns a typed list of all primary key columns in the table.
 * 
 * @return	A typed list of all primary key columns in the table.
 */
QList<const PrimaryKeyColumn*> Table::getPrimaryKeyColumnTypedList() const
{
	QList<const PrimaryKeyColumn*> primaryKeyColumns = QList<const PrimaryKeyColumn*>();
	for (const Column* column : getPrimaryKeyColumnList()) {
		primaryKeyColumns.append((PrimaryKeyColumn*) column);
	}
	return primaryKeyColumns;
}

/**
 * Returns a list of all foreign key columns in the table.
 * 
 * @return	A list of all foreign key columns in the table.
 */
QList<const Column*> Table::getForeignKeyColumnList() const
{
	QList<const Column*> foreignKeyColumns = QList<const Column*>();
	for (const Column* column : columns) {
		if (column->foreignColumn) foreignKeyColumns.append(column);
	}
	return foreignKeyColumns;
}

/**
 * Returns a typed list of all foreign key columns in the table.
 * 
 * @return	A typed list of all foreign key columns in the table.
 */
QList<const ForeignKeyColumn*> Table::getForeignKeyColumnTypedList() const
{
	QList<const ForeignKeyColumn*> foreignKeyColumns = QList<const ForeignKeyColumn*>();
	for (const Column* column : getForeignKeyColumnList()) {
		foreignKeyColumns.append((const ForeignKeyColumn*) column);
	}
	return foreignKeyColumns;
}

/**
 * Returns a list of all non-primary-key columns in the table.
 * 
 * @return	A list of all non-primary-key columns in the table.
 */
QList<const Column*> Table::getNonPrimaryKeyColumnList() const
{
	QList<const Column*> primaryKeyColumns = QList<const Column*>();
	for (const Column* column : columns) {
		if (!column->primaryKey) primaryKeyColumns.append(column);
	}
	return primaryKeyColumns;
}

/**
 * Returns a string listing all columns in the table.
 * 
 * @return	A string listing all columns in the table.
 */
QString Table::getColumnListString() const
{
	return getColumnListStringOf(getColumnList());
}

/**
 * Returns a string listing all primary key columns in the table.
 * 
 * @return	A string listing all primary key columns in the table.
 */
QString Table::getPrimaryKeyColumnListString() const
{
	return getColumnListStringOf(getPrimaryKeyColumnList());
}

/**
 * Returns the index of the given column.
 * 
 * @param column	The column whose index to return.
 * @return			The index of the given column.
 */
int Table::getColumnIndex(const Column& column) const
{
	return columns.indexOf(&column);
}

/**
 * Returns the column at the given index.
 * 
 * @param index	The index of the column to return.
 * @return		The column with the given index.
 */
const Column& Table::getColumnByIndex(int index) const
{
	assert(index >= 0 && index < columns.size());
	return *columns.at(index);
}



// BUFFER ACCESS

/**
 * Initializes the buffer with the contents from the database.
 * 
 * @param parent	The parent window.
 */
void Table::initBuffer(QWidget& parent)
{
	QList<QList<QVariant>*> newContents = getAllEntriesFromSql(parent);
	beginInsertRows(getNormalRootModelIndex(), 0, newContents.size() - 1);
	buffer.reset();
	buffer.setInitialNumberOfColumns(columns.size());
	for (QList<QVariant>* newRow : newContents) {
		buffer.appendRow(newRow);
	}
	endInsertRows();
}

/**
 * Resets the buffer.
 */
void Table::resetBuffer()
{
	beginRemoveRows(getNormalRootModelIndex(), 0, buffer.numRows() - 1);
	buffer.reset();
	endRemoveRows();
}

/**
 * Returns the number of rows in the table.
 * 
 * @return	The number of rows in the table.
 */
int Table::getNumberOfRows() const
{
	return buffer.numRows();
}

/**
 * Returns the row at the given index.
 * 
 * @param bufferRowIndex	The index of the row to return.
 * @return					The row at the given index.
 */
const QList<QVariant>* Table::getBufferRow(BufferRowIndex bufferRowIndex) const
{
	return buffer.getRow(bufferRowIndex);
}

/**
 * Collects indices of all rows in the table where the given column has the given value.
 * 
 * @param column	The column to check.
 * @param content	The value to check for.
 * @return			A list of all row indices in the table where the given column has the given value.
 */
QList<BufferRowIndex> Table::getMatchingBufferRowIndices(const Column& column, const QVariant& content) const
{
	assert(getColumnList().contains(&column));
	
	QList<BufferRowIndex> result = QList<BufferRowIndex>();
	for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(buffer.numRows()); rowIndex++) {
		if (column.getValueAt(rowIndex) == content) {
			result.append(rowIndex);
		}
	}
	return result;
}

/**
 * Finds the index of the row in the table where the given primary key columns have the given values.
 * 
 * @pre primaryKeyColumns contains all primary key columns of the table.
 * @pre primaryKeys contains one value for each of the primary key columns in the same order as them columns.
 * 
 * @param primaryKeyColumns	The primary key columns to check.
 * @param primaryKeys		The values to check for, in the same order as the columns.
 * @return					The index of the row in the table where the given primary key columns have the given values.
 */
BufferRowIndex Table::getMatchingBufferRowIndex(const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys) const
{
	int numPrimaryKeys = getPrimaryKeyColumnList().size();
	assert(primaryKeyColumns.size() == numPrimaryKeys);
	assert(primaryKeys.size() == numPrimaryKeys);
	
	for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(buffer.numRows()); bufferRowIndex++) {
		bool match = true;
		for (int i = 0; i < numPrimaryKeys; i++) {
			if (primaryKeyColumns.at(i)->getValueAt(bufferRowIndex) != ID_GET(primaryKeys.at(i))) {
				match = false;
				break;
			}
		}
		if (match) return bufferRowIndex;
	}
	assert(false);
	return BufferRowIndex();
}


/**
 * Prints the contents of the buffer to the console for debugging purposes.
 */
void Table::printBuffer() const
{
	qDebug() << "Printing buffer of" << name;
	QString header = "";
	for (const Column* column : getColumnList()) {
		header.append(column->name + "  ");
	}
	qDebug() << header;
	for (const QList<QVariant>* bufferRow : buffer) {
		QString rowString = "";
		for (int columnIndex = 0; columnIndex < getNumberOfColumns(); columnIndex++) {
			rowString.append(bufferRow->at(columnIndex).toString()).append("        ");
		}
		qDebug() << rowString;
	}
}



// MODIFICATIONS

/**
 * Adds a row to the table from a list of columns and a corresponding list of data.
 * 
 * @param parent			The parent window.
 * @param columnDataPairs	Pairs of columns and corresponding data to add.
 * @return					The index of the newly added row in the buffer.
 */
BufferRowIndex Table::addRow(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs)
{
	assert(db.currentlyAcceptingChanges());
	
	// Announce row insertion
	BufferRowIndex newItemBufferRowIndex = BufferRowIndex(buffer.numRows());
	beginInsertRows(getNormalRootModelIndex(),		newItemBufferRowIndex.get(), newItemBufferRowIndex.get());
	beginInsertRows(getNullableRootModelIndex(),	newItemBufferRowIndex.get(), newItemBufferRowIndex.get());
	
	// Add data to SQL database
	ItemID newRowID = addRowToSql(parent, columnDataPairs);
	
	// Update buffer
	QList<QVariant>* newBufferRow = new QList<QVariant>();
	for (const ColumnDataPair& columnDataPair : columnDataPairs) {
		newBufferRow->append(columnDataPair.second);
	}
	if (!isAssociative) {
		newBufferRow->insert(0, newRowID.asQVariant());
	}
	buffer.appendRow(newBufferRow);
	
	// Announce end of row insertion
	endInsertRows();
	db.rowsAdded(*this, { newItemBufferRowIndex });
	
	return newItemBufferRowIndex;
}

/**
 * Updates a cell in the table, specified by primary key and column, provided it is a normal table.
 * 
 * @pre The table is not associative.
 * 
 * @param parent		The parent window.
 * @param primaryKey	The primary key of the row to update.
 * @param column		The column to update.
 * @param data			The new data for the cell.
 */
void Table::updateCellInNormalTable(QWidget& parent, const ValidItemID primaryKey, const Column& column, const QVariant& data)
{
	assert(db.currentlyAcceptingChanges());
	assert(!isAssociative);
	
	QList<const Column*> primaryKeyColumns = getPrimaryKeyColumnList();
	assert(primaryKeyColumns.size() == 1);
	assert(&column.table == this);
	
	// Update cell in SQL database
	updateCellOfNormalTableInSql(parent, primaryKey, column, data);
	
	// Update buffer
	BufferRowIndex bufferRowIndex = getMatchingBufferRowIndex(primaryKeyColumns, { primaryKey });
	buffer.replaceCell(bufferRowIndex, column.getIndex(), data);
	
	// Announce changed data
	QModelIndex updateIndexNormal	= index(bufferRowIndex.get(), column.getIndex(), getNormalRootModelIndex());
	QModelIndex updateIndexNullable	= index(bufferRowIndex.get(), column.getIndex(), getNullableRootModelIndex());
	const QList<int> updatedDatumRoles = { column.type == Bit ? Qt::CheckStateRole : Qt::DisplayRole };
	Q_EMIT dataChanged(updateIndexNormal, updateIndexNormal, updatedDatumRoles);
	Q_EMIT dataChanged(updateIndexNullable, updateIndexNullable, updatedDatumRoles);
	db.columnDataChanged(&column);
}

/**
 * Updates rows in the table, specified by buffer indices, provided it is a normal table.
 * 
 * @pre The table is not associative.
 * 
 * @param parent			The parent window.
 * @param bufferIndices		The buffer indices of the rows to update.
 * @param columnDataPairs	Pairs of columns and corresponding data to update.
 */
void Table::updateRowsInNormalTable(QWidget& parent, const QSet<BufferRowIndex>& bufferIndices, const QList<ColumnDataPair>& columnDataPairs)
{
	assert(db.currentlyAcceptingChanges());
	assert(!isAssociative);
	
	const Column* const primaryKeyColumn = getPrimaryKeyColumnList().at(0);
	
	BufferRowIndex minBufferRow = BufferRowIndex(getNumberOfRows());
	BufferRowIndex maxBufferRow = BufferRowIndex(0);
	
	for (const BufferRowIndex& bufferIndex : bufferIndices) {
		const ValidItemID primaryKey = VALID_ITEM_ID(primaryKeyColumn->getValueAt(bufferIndex));
		
		// Update row in SQL database
		updateRowInSql(parent, primaryKey, columnDataPairs);
		
		// Update buffer
		for (const auto& [column, data] : columnDataPairs) {
			buffer.replaceCell(bufferIndex, column->getIndex(), data);
		}
		
		if (bufferIndex < minBufferRow) minBufferRow = bufferIndex;
		if (bufferIndex > maxBufferRow) maxBufferRow = bufferIndex;
	}
	
	// Announce changed data
	QModelIndex updateIndexLeft		= index(minBufferRow.get(), 0,							getNormalRootModelIndex());
	QModelIndex updateIndexRight	= index(maxBufferRow.get(), getNumberOfColumns() - 1,	getNormalRootModelIndex());
	const QList<int> updatedDatumRoles = { Qt::CheckStateRole, Qt::DisplayRole };
	Q_EMIT dataChanged(updateIndexLeft, updateIndexRight, updatedDatumRoles);
	for (const auto& [column, _] : columnDataPairs) {
		db.columnDataChanged(column);
	}
	
}

/**
 * Updates a row in the table, specified by primary key, provided it is a normal table.
 * 
 * Removes all column data pairs from the list which do not represent actual changes in the
 * database.
 * 
 * @pre The table is not associative.
 * 
 * @param parent			The parent window.
 * @param primaryKey		The primary key of the row to update.
 * @param columnDataPairs	Pairs of columns and corresponding data to update.
 */
void Table::updateRowInNormalTable(QWidget& parent, const ValidItemID primaryKey, QList<ColumnDataPair>& columnDataPairs)
{
	assert(db.currentlyAcceptingChanges());
	assert(!isAssociative);
	
	QList<const Column*> primaryKeyColumns = getPrimaryKeyColumnList();
	assert(primaryKeyColumns.size() == 1);
	
	const BufferRowIndex bufferIndex = getMatchingBufferRowIndex(primaryKeyColumns, { primaryKey });
	
	// Drop column data pairs with no change
	for (int i = columnDataPairs.size() - 1; i >= 0; i--) {
		const Column* const column = columnDataPairs.at(i).first;
		const QVariant newValue = columnDataPairs.at(i).second;
		const QVariant currentValue = buffer.getCell(bufferIndex, column->getIndex());
		if (currentValue == newValue) columnDataPairs.remove(i);
	}
	if (columnDataPairs.isEmpty()) return;
	
	updateRowsInNormalTable(parent, { bufferIndex }, columnDataPairs);
}

/**
 * Removes a row from the table, specified by primary keys.
 * 
 * @param parent			The parent window.
 * @param primaryKeyColumns	The primary key columns.
 * @param primaryKeys		The primary keys of the row to remove, in the same order as the columns.
 */
void Table::removeRow(QWidget& parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys)
{
	assert(db.currentlyAcceptingChanges());
	
	int numPrimaryKeys = getNumberOfPrimaryKeyColumns();
	assert(primaryKeyColumns.size() == numPrimaryKeys);
	assert(primaryKeys.size() == numPrimaryKeys);
	
	BufferRowIndex bufferRowIndex = getMatchingBufferRowIndex(primaryKeyColumns, primaryKeys);
	assert(bufferRowIndex.isValid());
	
	// Announce row removal
	beginRemoveRows(getNormalRootModelIndex(), bufferRowIndex.get(), bufferRowIndex.get());
	
	// Remove row from SQL database
	removeRowFromSql(parent, primaryKeyColumns, primaryKeys);
	
	// Update buffer
	buffer.removeRow(bufferRowIndex);
	
	// Announce end of row removal
	endRemoveRows();
	db.rowsRemoved(*this, { bufferRowIndex });
}

/**
 * Removes all rows from the table where the given column has one of the given values.
 * 
 * @param parent	The parent window.
 * @param column	The column to check.
 * @param keys		The values to check for.
 */
void Table::removeMatchingRows(QWidget& parent, const Column& column, const QSet<ValidItemID>& keys)
{
	assert(db.currentlyAcceptingChanges());
	assert(getColumnList().contains(&column));
	assert(column.isKey());
	assert(!keys.isEmpty());
	
	for (const ValidItemID& key : keys) {
		// Remove rows from SQL database
		removeMatchingRowsFromSql(parent, column, key);
		
		// Update buffer
		QList<BufferRowIndex> bufferRowIndices = getMatchingBufferRowIndices(column, key.asQVariant());
		if (bufferRowIndices.isEmpty()) continue;
		
		auto iter = bufferRowIndices.constEnd();
		while (iter-- != bufferRowIndices.constBegin()) {
			// Announce row removal
			BufferRowIndex bufferRowIndex = *iter;
			beginRemoveRows(getNormalRootModelIndex(), bufferRowIndex.get(), bufferRowIndex.get());
			
			buffer.removeRow(bufferRowIndex);
			
			// Announce end of row removal
			endRemoveRows();
			db.rowsRemoved(*this, { bufferRowIndex });
		}
	}
}

/**
 * Removes all rows from the table where the given column has the given value.
 * 
 * @param parent	The parent window.
 * @param column	The column to check.
 * @param key		The value to check for.
 */
void Table::removeMatchingRows(QWidget& parent, const Column& column, ValidItemID key)
{
	assert(db.currentlyAcceptingChanges());
	
	QSet<ValidItemID> keys = { key };
	return removeMatchingRows(parent, column, keys);
}



// SQL

/**
 * Creates the table in the SQL database.
 * 
 * Only needed when creating a new database or updating the project file version.
 * 
 * @param parent	The parent window.
 */
void Table::createTableInSql(QWidget& parent)
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
	
	if (!query.exec(queryString)) {
		displayError(parent, query.lastError(), queryString);
	}
}

/**
 * Runs a SQL query for all data in the table and returns the result as a two-dimensional list of
 * QVariants.
 * 
 * @param parent	The parent window.
 * @return			A two-dimensional list of QVariants containing the response to the SQL query.
 */
QList<QList<QVariant>*> Table::getAllEntriesFromSql(QWidget& parent) const
{
	QString queryString = QString(
			"SELECT " + getColumnListString() +
			"\nFROM " + name
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	QList<QList<QVariant>*> result = QList<QList<QVariant>*>();
	
	if (!query.exec(queryString)) {
		displayError(parent, query.lastError(), queryString);
	}
	
	QList<const Column*> columns = getColumnList();
	
	int rowIndex = 0;
	while (query.next()) {
		result.append(new QList<QVariant>());
		int columnIndex = 0;
		for (const Column* column : columns) {
			QVariant value = query.value(columnIndex);
			switch (column->type) {
			case Bit:		value = value.toBool();	break;
			case Date:		value = value.toDate();	break;
			case Time:		value = value.toTime();	break;
			case String:	if (value.toString().isEmpty()) value = QVariant();	break;
			default: break;
			}
			assert(column->nullable || !value.isNull());
			if (value.isNull()) value = QVariant();
			result.at(rowIndex)->append(value);
			columnIndex++;
		}
		rowIndex++;
	}
	
	return result;
}

/**
 * Adds a new row to the table in the SQL database.
 * 
 * @param parent			The parent window.
 * @param columnDataPairs	Pairs of columns and corresponding data to add.
 * @return					The ID of the newly added row.
 */
ValidItemID Table::addRowToSql(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs)
{
	QString questionMarks = "";
	for (int i = 0; i < columnDataPairs.size(); i++) {
		questionMarks = questionMarks + ((i == 0) ? "?" : ", ?");
	}
	QString queryString = QString(
			"INSERT INTO " + name + "(" + getColumnListStringFrom(columnDataPairs) + ")" +
			"\nVALUES(" + questionMarks + ")"
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString)) {
		displayError(parent, query.lastError(), queryString);
	}
	for (const ColumnDataPair& columnDataPair : columnDataPairs) {
		query.addBindValue(columnDataPair.second);
	}
	
	if (!query.exec()) {
		displayError(parent, query.lastError(), queryString);
	}
	
	ValidItemID newRowID = VALID_ITEM_ID(query.lastInsertId().toInt());
	return newRowID;
}

/**
 * Updates a cell of a normal table in the SQL database.
 * 
 * @param parent		The parent window.
 * @param primaryKey	The primary key of the row to update.
 * @param column		The column to update.
 * @param data			The new data for the cell.
 */
void Table::updateCellOfNormalTableInSql(QWidget& parent, const ValidItemID primaryKey, const Column& column, const QVariant& data)
{
	assert(!isAssociative);
	
	QList<const Column*> primaryKeyColumns = getPrimaryKeyColumnList();
	const Column& primaryKeyColumn = *primaryKeyColumns.first();
	
	QString queryString = QString(
			"UPDATE " + name +
			"\nSET " + column.name + " = ?" +
			"\nWHERE " + primaryKeyColumn.name + " = " + QString::number(ID_GET(primaryKey))
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString))
		displayError(parent, query.lastError(), queryString);
	query.addBindValue(data);
	
	if (!query.exec()) {
		displayError(parent, query.lastError(), queryString);
	}
}

/**
 * Updates a row in the table in the SQL database.
 * 
 * @param parent			The parent window.
 * @param primaryKey		The primary key of the row to update.
 * @param columnDataPairs	Pairs of columns and corresponding data to update.
 */
void Table::updateRowInSql(QWidget& parent, const ValidItemID primaryKey, const QList<ColumnDataPair>& columnDataPairs)
{
	assert(!columnDataPairs.isEmpty());
	
	QList<const Column*> primaryKeyColumns = getPrimaryKeyColumnList();
	const Column& primaryKeyColumn = *primaryKeyColumns.first();
	
	QString setString = "";
	for (const auto& [column, data] : columnDataPairs) {
		if (!setString.isEmpty()) setString.append(", ");
		setString.append(column->name).append(" = ?");
	}
	QString queryString = QString(
			"UPDATE " + name +
			"\nSET " + setString +
			"\nWHERE " + primaryKeyColumn.name + " = " + QString::number(ID_GET(primaryKey))
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString))
		displayError(parent, query.lastError(), queryString);
	for (int i = 0; i < columnDataPairs.size(); i++) {
		query.addBindValue(columnDataPairs.at(i).second);
	}
	
	if (!query.exec()) {
		displayError(parent, query.lastError(), queryString);
	}
}

/**
 * Removes a row from the table in the SQL database.
 * 
 * @param parent			The parent window.
 * @param primaryKeyColumns	The primary key columns.
 * @param primaryKeys		The primary keys of the row to remove, in the same order as the columns.
 */
void Table::removeRowFromSql(QWidget& parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys)
{
	assert(!primaryKeys.isEmpty());
	
	QString condition = "";
	for (int i = 0; i < primaryKeys.size(); i++) {
		if (i > 0) condition.append(" AND ");
		const Column& column = *primaryKeyColumns.at(i);
		assert(&column.table == this && column.isPrimaryKey());
		
		condition.append(column.name + " = " + QString::number(ID_GET(primaryKeys.at(i))));
	}
	QString queryString = QString(
			"DELETE FROM " + name +
			"\nWHERE " + condition
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	
	if (!query.exec(queryString)) {
		displayError(parent, query.lastError(), queryString);
	}
}

/**
 * Removes all rows from the table in the SQL database where the given column has the given value.
 * 
 * @param parent	The parent window.
 * @param column	The column to check.
 * @param key		The value to check for.
 */
void Table::removeMatchingRowsFromSql(QWidget& parent, const Column& column, ValidItemID key)
{
	assert(getColumnList().contains(&column));
	
	QString queryString = QString(
			"DELETE FROM " + name +
			"\nWHERE " + column.name + " = " + QString::number(ID_GET(key))
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	
	if (!query.exec(queryString)) {
		displayError(parent, query.lastError(), queryString);
	}
}


/**
 * Returns a string listing the given columns' names from a list of column-data pairs.
 * 
 * @param columnDataPairs	A list of pairs of columns and associated values.
 * @return					A comma-separated list of the columns' names.
 */
QString Table::getColumnListStringFrom(const QList<ColumnDataPair>& columnDataPairs)
{
	QString listString = QString();
	for (const auto& [column, data] : columnDataPairs) {
		if (!listString.isEmpty()) listString.append(", ");
		listString.append(column->name);
	}
	return listString;
}



// QABSTRACTMODEL IMPLEMENTATION

/**
 * For the QAbstractItemModel implementation, creates a QModelIndex for the given row and column.
 * 
 * @param row		The row index.
 * @param column	The column index.
 * @param parent	The parent model index.
 * @return			The QModelIndex for the given row and column.
 */
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

/**
 * For the QAbstractItemModel implementation, returns the parent of the given model index.
 * 
 * @param index	A model index.
 * @return		The parent model index of the given model index.
 */
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

/**
 * For the QAbstractItemModel implementation, returns the number of rows at the given parent model
 * index.
 * 
 * @param parent	The parent model index.
 * @return			The number of rows at the given parent model index.
 */
int Table::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid()) return 2;
	int numberActualRows = buffer.numRows();
	if (parent.row() == 0) {
		return numberActualRows;
	} else {
		return numberActualRows + 1;
	}
}

/**
 * For the QAbstractItemModel implementation, returns the number of columns at the given parent
 * model index.
 * 
 * @param parent	The parent model index.
 * @return			The number of columns at the given parent model index.
 */
int Table::columnCount(const QModelIndex& parent) const
{
	if (!parent.isValid()) return 1;
	return getNumberOfColumns();
}

/**
 * For the QAbstractItemModel implementation, returns the data for the given role at the given
 * model index.
 * 
 * Delegates to multiData() implementations in subclasses NormalTable and AssociativeTable.
 * 
 * @param index	The model index.
 * @param role	The role of the data to return.
 * @return		The data at the given model index.
 */
QVariant Table::data(const QModelIndex& index, int role) const
{
	QModelRoleData roleData(role);
	multiData(index, roleData);
	return roleData.data();
}

/**
 * For the QAbstractItemModel implementation, returns the header data for the given role, section
 * index and orientation.
 * 
 * @param section		The section index.
 * @param orientation	The orientation of the header (horizontal or vertical).
 * @param role			The role of the header data to return.
 */
QVariant Table::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (orientation) {
	case Qt::Orientation::Vertical: {
		switch (role) {
		case Qt::TextAlignmentRole:	return Qt::AlignRight;
		case Qt::DisplayRole:		return section + 1;
		default:					return QVariant();
		}
	}
	case Qt::Orientation::Horizontal: {
		switch (role) {
		case Qt::DisplayRole: {
			QString result = getColumnByIndex(section).uiName;
			if (result.isEmpty()) result = getColumnByIndex(section).name;
			return result;
		}
		default: return QVariant();
		}
	}
	default: assert(false);
	}
	return QVariant();
}


/** Role index to use for requesting the primary key of an entry via data(). */
const int Table::PrimaryKeyRole = -1;

/**
 * Creates a QModelIndex for the root of the index space for normal table views, where the first
 * element has row index 0.
 * 
 * @return	The root model index for normal mode.
 */
QModelIndex Table::getNormalRootModelIndex() const
{
	return index(0, 0);
}

/**
 * Creates a QModelIndex for the root of the index space for nullable table views, where the first
 * element has row index 1 and there is an additional element at row index 0, representing the
 * absence of a value ("none").
 * 
 * @return	The root model index for nullable mode.
 */
QModelIndex Table::getNullableRootModelIndex() const
{
	return index(1, 0);
}
