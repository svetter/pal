#include "normal_table.h"

#include "db_error.h"

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>



const int NormalTable::PrimaryKeyRole = -1;



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

QString NormalTable::getNonPrimaryKeyColumnListString() const
{
	return getColumnListStringOf(getNonPrimaryKeyColumnList());
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


int NormalTable::getNumberOfRows() const
{
	return buffer->size();
}



int NormalTable::addRow(QWidget* parent, const QList<QVariant>& data)
{
	assert(data.size() == getNumberOfNonPrimaryKeyColumns());
	
	int currentNumRows = buffer->size();
	beginInsertRows(index(0, 0, QModelIndex()), currentNumRows, currentNumRows);
	int newRowIndex = Table::addRow(parent, data, getNonPrimaryKeyColumnList());
	endInsertRows();
	
	// TODO write-through to database
	
	return newRowIndex;
}

void NormalTable::removeRow(QWidget* parent, int primaryKey)
{
	// TODO
}



QModelIndex NormalTable::index(int row, int column, const QModelIndex& parent) const
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
		
		if (role == PrimaryKeyRole) {
			if (rowIndex >= 0) {
				roleData.setData(buffer->at(rowIndex)->at(primaryKeyColumn->getIndex()));
			} else {
				roleData.setData(-1);
			}
			continue;
		}
		Column* column = getColumnByIndex(columnIndex);
		
		QVariant bufferValue = (rowIndex < 0) ? QVariant() : buffer->at(rowIndex)->at(columnIndex);
		QVariant result = QVariant();
		
		switch (column->getType()) {
		case integer:
			switch (role) {
			case Qt::DisplayRole:
				if (rowIndex == -1) {
					result = QVariant(noneString);
				} else {
					result = bufferValue;
				}
				break;
			case Qt::TextAlignmentRole:
				result = Qt::AlignRight;
				break;
			}
			break;
		case bit:
			switch (role) {
			case Qt::CheckStateRole:
				result = bufferValue.toBool() ? Qt::Checked : Qt::Unchecked;
				break;
			case Qt::TextAlignmentRole:
				result = Qt::AlignCenter;
				break;
			}
			break;
		case varchar:
			if (role != Qt::DisplayRole) break;
			if (rowIndex == -1) {
				result = QVariant(noneString);
			} else {
				result = bufferValue;
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
			assert(false);
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