#include "table.h"

#include "db_error.h"

#include <QSqlQuery>



Table::Table(QString name, QString uiName, bool isAssociative) :
		name(name),
		uiName(uiName),
		associative(isAssociative),
		buffer(nullptr)
{}

Table::~Table()
{
	deleteBuffer();
}


QString Table::getName() const
{
	return name;
}
QString Table::getUIName() const
{
	return uiName;
}

bool Table::isAssociative() const
{
	return associative;
}



void Table::deleteBuffer()
{
	if (buffer) {
		qDeleteAll(*buffer);
		delete buffer;
	}
}

void Table::initBuffer(QWidget* parent)
{
	deleteBuffer();
	buffer = getAllEntriesFromSql(parent);
}

const QList<QVariant>* Table::getBufferRow(int rowIndex) const
{
	return buffer->at(rowIndex);
}

QSet<int> Table::getMatchingBufferRowIndices(const Column* column, const QVariant& content) const
{
	assert(column->getTable() == this);
	QSet<int> result = QSet<int>();
	int rowIndex = 0;
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(column->getIndex()) == content) {
			result.insert(rowIndex);
		}
		rowIndex++;
	}
	return result;
}

void Table::printBuffer() const
{
	qDebug() << "Printing buffer of" << name;
	QString header = "";
	for (const Column* column : getColumnList()) {
		header.append(column->getName() + "  ");
	}
	qDebug() << header;
	for (QList<QVariant>* bufferRow : *buffer) {
		QString rowString = "";
		for (int columnIndex = 0; columnIndex < getNumberOfColumns(); columnIndex++) {
			rowString.append(bufferRow->at(columnIndex).toString()).append("        ");
		}
		qDebug() << rowString;
	}
}



QString Table::getColumnListString() const
{
	return getColumnListStringOf(getColumnList());
}

int Table::getColumnIndex(const Column* column) const
{
	return getColumnList().indexOf(column);
}

const Column* Table::getColumnByIndex(int index) const
{
	assert(index >= 0 && index < getNumberOfColumns());
	return getColumnList().at(index);
}



int Table::addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data)
{	
	int numColumns = columns.size();
	assert(!isAssociative() && numColumns == (getNumberOfColumns() - 1) || isAssociative() && numColumns == getNumberOfColumns());
	assert(data.size() == numColumns);
	
	// Add data to SQL database
	int newRowID = addRowToSql(parent, columns, data);
	
	// Update buffer
	QList<QVariant>* newBufferRow = new QList<QVariant>(data);
	newBufferRow->insert(0, newRowID);
	buffer->append(newBufferRow);
	
	return buffer->size() - 1;
}

void Table::removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys)
{
	int numPrimaryKeys = getPrimaryKeyColumnList().size();
	assert(primaryKeyColumns.size() == numPrimaryKeys);
	assert(primaryKeys.size() == numPrimaryKeys);
	
	// Remove row from SQL database
	removeRowFromSql(parent, primaryKeyColumns, primaryKeys);
	
	// Update buffer
	int bufferRowIndex = getMatchingBufferRowIndex(primaryKeyColumns, primaryKeys);
	const QList<QVariant>* rowToRemove = getBufferRow(bufferRowIndex);
	buffer->remove(bufferRowIndex);
	delete rowToRemove;
}



QList<QList<QVariant>*>* Table::getAllEntriesFromSql(QWidget* parent) const
{
	QString queryString = QString(
			"SELECT " + getColumnListString() +
			"\nFROM " + name
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	QList<QList<QVariant>*>* buffer = new QList<QList<QVariant>*>();
	
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
	
	QList<const Column*> columns = getColumnList();
	
	int rowIndex = 0;
	while (query.next()) {
		buffer->append(new QList<QVariant>());
		int columnIndex = 0;
		for (auto columnIter = columns.constBegin(); columnIter!= columns.constEnd(); columnIter++) {
			QVariant value = query.value(columnIndex);
			const Column* column = *columnIter;
			assert(column->isNullable() || !value.isNull());
			buffer->at(rowIndex)->append(value);
			columnIndex++;
		}
		rowIndex++;
	}
	
	if (buffer->empty())
		displayError(parent, "Couldn't read record from SQL query", queryString);
	return buffer;
}

int Table::addRowToSql(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data)
{
	assert(columns.size() == data.size());
	
	QString questionMarks = "";
	for (int i = 0; i < columns.size(); i++) {
		questionMarks = questionMarks + ((i == 0) ? "?" : ", ?");
	}
	QString queryString = QString(
			"INSERT INTO " + getName() + "(" + getColumnListStringOf(columns) + ")" +
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

void Table::removeRowFromSql(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys)
{
	QString condition = "";
	for (int i = 0; i < primaryKeys.size(); i++) {
		if (i > 0) condition.append(" AND ");
		const Column* column = primaryKeyColumns.at(i);
		int key = primaryKeys.at(i).toInt();
		assert(column->getTable() == this && column->isPrimaryKey());
		assert(key > 0);
		
		condition.append(column->getName() + " = " + QString::number(key));
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



int Table::getMatchingBufferRowIndex(const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys) const
{
	int numPrimaryKeys = getPrimaryKeyColumnList().size();
	assert(primaryKeyColumns.size() == numPrimaryKeys);
	assert(primaryKeys.size() == numPrimaryKeys);
	
	for (int rowIndex = 0; rowIndex < buffer->size(); rowIndex++) {
		const QList<QVariant>* row = getBufferRow(rowIndex);
		bool match = true;
		for (int i = 0; i < numPrimaryKeys; i++) {
			if (row->at(i) != primaryKeys.at(i)) {
				match = false;
				break;
			}
		}
		if (match) return rowIndex;
	}
	assert(false);
	return -1;
}



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
	int numberActualRows = buffer->size();
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
	if (role != Qt::DisplayRole) return QVariant();
	if (orientation != Qt::Orientation::Horizontal) return QVariant(section + 1);
	QString result = getColumnByIndex(section)->uiName;
	if (result.isEmpty()) result = getColumnByIndex(section)->name;
	return result;
}

QModelIndex Table::getNormalRootModelIndex() const
{
	return index(0, 0);
}
