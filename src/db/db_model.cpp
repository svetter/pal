#include "db_model.h"
#include "qsqlerror.h"
#include "src/db/db_error.h"

#include <assert.h>
#include <QSqlQuery>



Column::Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, Column* foreignKey, Table* inTable) :
		name(name),
		uiName(uiName),
		type(type),
		primaryKey(primaryKey),
		foreignKey(foreignKey),
		nullable(nullable),
		inTable(inTable)
{
	assert(name.compare(QString("ID"), Qt::CaseInsensitive) != 0);
	assert(inTable->isAssociative() == (primaryKey && foreignKey));
	if (primaryKey)					assert(!nullable);
	if (primaryKey || foreignKey)	assert(type == integer && name.endsWith("ID"));
	if (name.endsWith("ID"))		assert(primaryKey || foreignKey);
}


QString Column::getName()
{
	return name;
}

QString Column::getUIName()
{
	return uiName;
}

DataType Column::getType()
{
	return type;
}

bool Column::isPrimaryKey()
{
	return primaryKey;
}

bool Column::isForeignKey()
{
	return foreignKey;
}

Column* Column::getReferencedForeignColumn()
{
	return foreignKey;
}

bool Column::isNullable()
{
	return nullable;
}

Table* Column::getTable()
{
	return inTable;
}


int Column::getIndex()
{
	return inTable->getColumnIndex(this);
}



QString getColumnListString(QList<Column*> columns)
{
	QString result = "";
	bool first = true;
	for (auto iter = columns.begin(); iter != columns.end(); iter++) {
		result = result + (first ? "" : ", ") + (*iter)->getName();
		first = false;
	}
	return result;
}



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


QString Table::getName()
{
	return name;
}
QString Table::getUIName()
{
	return uiName;
}

bool Table::isAssociative()
{
	return associative;
}



int Table::getColumnIndex(Column* column) const
{
	return getColumnList().indexOf(column);
}

QString Table::getColumnListString() const
{
	return ::getColumnListString(getColumnList());
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
	buffer = getAllEntries(parent);
}

void Table::ensureBuffer(QWidget* parent)
{
	if (!buffer) initBuffer(parent);
}

QList<QList<QVariant>*>* Table::getAllEntries(QWidget* parent) const
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
	
	QList<Column*> columns = getColumnList();
	
	int rowIndex = 0;
	while (query.next()) {
		buffer->append(new QList<QVariant>());
		int columnIndex = 0;
		for (auto columnIter = columns.constBegin(); columnIter!= columns.constEnd(); columnIter++) {
			QVariant value = query.value(columnIndex);
			Column* column = *columnIter;
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



NormalTable::NormalTable(QString name, QString itemNameSingularLowercase, QString uiName, QString noneString) :
		Table(name, uiName, false),
		noneString(noneString),
		primaryKeyColumn(new Column(itemNameSingularLowercase + "ID", QString(), DataType::integer, false, true, nullptr, this)),
		nonPrimaryColumns(QList<Column*>())
{}

NormalTable::~NormalTable()
{
	delete primaryKeyColumn;
	for (auto iter = nonPrimaryColumns.begin(); iter != nonPrimaryColumns.end(); iter++) {
		delete *iter;
	}
}


void NormalTable::addColumn(Column* column)
{
	nonPrimaryColumns.append(column);
}


Column* NormalTable::getPrimaryKeyColumn() const
{
	return primaryKeyColumn;
}

QList<Column*> NormalTable::getColumnList() const
{
	QList<Column*> result = { primaryKeyColumn };
	result.append(nonPrimaryColumns);
	return result;
}

QList<Column*> NormalTable::getNonPrimaryKeyColumnList() const
{
	return QList<Column*>(nonPrimaryColumns);
}

int NormalTable::getNumberOfColumns() const
{
	return nonPrimaryColumns.size() + 1;
}

int NormalTable::getNumberOfNonPrimaryKeyColumns() const
{
	return nonPrimaryColumns.size();
}

Column* NormalTable::getColumnByIndex(int index) const
{
	assert(index >= 0 && index < getNumberOfColumns());
	if (index == 0) return primaryKeyColumn;
	return nonPrimaryColumns.at(index - 1);
}

int NormalTable::getBufferIndexForPrimaryKey(int primaryKey) const
{
	int index = 0;
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(0) == primaryKey) return index;
		index++;
	}
	return -1;
}


int NormalTable::getNumberOfEntries(QWidget* parent)
{
	QString queryString = QString(
			"SELECT COUNT(" + getPrimaryKeyColumn()->getName() + ")" +
			"\nFROM " + getName()
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
	if (!query.next()) {
		displayError(parent, "Couldn't read record from SQL query \"" + queryString + "\"", queryString);
	}
	QVariant variantValue = query.value(0);
	if (!variantValue.isValid())
		displayError(parent, "Received invalid QVariant from query", queryString);
	if (query.next())
		displayError(parent, "More than one record returned for query", queryString);
	bool intConversionOk;
	int intValue = variantValue.toInt(&intConversionOk);
	if (!intConversionOk)
		displayError(parent, "Conversion to int failed for result from query", queryString);
	return intValue;
}


QModelIndex NormalTable::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent)) {
		return QModelIndex();
	}
	if (!parent.isValid()) {
		return createIndex(row, column, -(row + 1));
	}
	return createIndex(row, column, parent.row());
}

QModelIndex NormalTable::parent(const QModelIndex& index) const
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

int NormalTable::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid()) return 2;
	int numberActualRows = buffer->size();
	if (parent.row() == 0) {
		return numberActualRows;
	} else {
		return numberActualRows + 1;
	}
}

int NormalTable::columnCount(const QModelIndex& parent) const
{
	if (!parent.isValid()) return 1;
	return getNumberOfColumns();
}

void NormalTable::multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const
{
	for (QModelRoleData& roleData : roleDataSpan) {
		int role = roleData.role();
		int rowIndex;
		if (index.parent().row() == 0) {
			rowIndex = index.row();
		} else {
			rowIndex = index.row() - 1;
		}
		int columnIndex = index.column();
		Column* column = getColumnByIndex(columnIndex);
		
		QVariant bufferValue = buffer->at(rowIndex)->at(columnIndex);
		
		QVariant result = QVariant();
		switch (column->getType()) {
		case bit:
			if (role == Qt::CheckStateRole) {
				result = bufferValue.toBool() ? Qt::Checked : Qt::Unchecked;
			}
			break;
		case date:
			if (role == Qt::DisplayRole) {
				result = QVariant(bufferValue.toDate().toString("dd.MM.yyyy"));
			}
			break;
		case time_:
			if (role == Qt::DisplayRole) {
				result = QVariant(bufferValue.toTime().toString("HH:mm"));
			}
			break;
		default:
			if (role != Qt::DisplayRole) break;
			if (rowIndex == -1) {
				result = QVariant(noneString);
			} else {
				result = bufferValue;
			}
			break;
		}
		roleData.setData(result);
	}
}

QVariant NormalTable::data(const QModelIndex& index, int role) const
{
	QModelRoleData roleData(role);
	multiData(index, roleData);
	return roleData.data();
}

QVariant NormalTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole) return QVariant();
	if (orientation != Qt::Orientation::Horizontal) return QVariant(section + 1);
	QString result = getColumnByIndex(section)->uiName;
	if (result.isEmpty()) result = getColumnByIndex(section)->name;
	return result;
}


QModelIndex NormalTable::getNormalRootModelIndex() const
{
	return index(0, 0);
}

QModelIndex NormalTable::getNullableRootModelIndex() const
{
	return index(1, 0);
}



AssociativeTable::AssociativeTable(QString name, Column* foreignKeyColumn1, Column* foreignKeyColumn2) :
		Table(name, QString(), true),
		column1(new Column(foreignKeyColumn1->name, foreignKeyColumn1->uiName, DataType::integer, false, true, foreignKeyColumn1, this)),
		column2(new Column(foreignKeyColumn2->name, foreignKeyColumn2->uiName, DataType::integer, false, true, foreignKeyColumn2, this))
{
	assert(foreignKeyColumn1->primaryKey && foreignKeyColumn1->type == DataType::integer);
	assert(foreignKeyColumn2->primaryKey && foreignKeyColumn2->type == DataType::integer);
}

AssociativeTable::~AssociativeTable()
{
	delete column1;
	delete column2;
}

Column* AssociativeTable::getColumn1()
{
	return column1;
}

Column* AssociativeTable::getColumn2()
{
	return column2;
}

int AssociativeTable::getNumberOfColumns() const
{
	return 2;
}

QList<Column*> AssociativeTable::getColumnList() const
{
	return { column1, column2 };
}
