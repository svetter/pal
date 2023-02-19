#include "normal_table.h"

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>



NormalTable::NormalTable(QString name, QString uiName, QString primaryKeyColumnName) :
		Table(name, uiName, false),
		primaryKeyColumn(new Column(primaryKeyColumnName, QString(), DataType::integer, false, true, nullptr, this)),
		nonPrimaryColumns(QList<const Column*>())
{}

NormalTable::~NormalTable()
{
	delete primaryKeyColumn;
	for (auto iter = nonPrimaryColumns.begin(); iter != nonPrimaryColumns.end(); iter++) {
		delete *iter;
	}
}



void NormalTable::addColumn(const Column* column)
{
	nonPrimaryColumns.append(column);
}


const Column* NormalTable::getPrimaryKeyColumn() const
{
	return primaryKeyColumn;
}

QList<const Column*> NormalTable::getColumnList() const
{
	QList<const Column*> result = getNonPrimaryKeyColumnList();
	result.insert(0, primaryKeyColumn);
	return result;
}

QList<const Column*> NormalTable::getPrimaryKeyColumnList() const
{
	return { primaryKeyColumn };
}

QList<const Column*> NormalTable::getNonPrimaryKeyColumnList() const
{
	QList<const Column*> result = QList<const Column*>();
	for (auto iter = nonPrimaryColumns.constBegin(); iter != nonPrimaryColumns.constEnd(); iter++) {
		result.append(*iter);
	}
	return result;
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

int NormalTable::getBufferIndexForPrimaryKey(ValidItemID primaryKey) const
{
	int index = 0;
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(0) == primaryKey.get()) return index;
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
	beginInsertRows(getNormalRootModelIndex(), currentNumRows, currentNumRows);
	int newRowIndex = Table::addRow(parent, getNonPrimaryKeyColumnList(), data);
	endInsertRows();
	
	return newRowIndex;
}

void NormalTable::removeRow(QWidget* parent, ValidItemID primaryKey)
{
	int bufferRowIndex = getBufferIndexForPrimaryKey(primaryKey);
	beginRemoveRows(getNormalRootModelIndex(), bufferRowIndex, bufferRowIndex);
	Table::removeRow(parent, getPrimaryKeyColumnList(), { primaryKey.get() });
	endRemoveRows();
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
		const Column* column = getColumnByIndex(columnIndex);
		
		QVariant bufferValue = (rowIndex < 0) ? QVariant() : buffer->at(rowIndex)->at(columnIndex);
		QVariant result = QVariant();
		
		switch (column->getType()) {
		case integer:
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
				result = QVariant(getNoneString());
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



const int NormalTable::PrimaryKeyRole = -1;

QModelIndex NormalTable::getNullableRootModelIndex() const
{
	return index(1, 0);
}
