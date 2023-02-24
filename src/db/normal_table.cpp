#include "normal_table.h"

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>



NormalTable::NormalTable(QString name, QString uiName, QString primaryKeyColumnName) :
		Table(name, uiName, false),
		primaryKeyColumn(new Column(primaryKeyColumnName, QString(), DataType::integer, false, true, nullptr, this))
{}

NormalTable::~NormalTable()
{}



// BUFFER ACCESS

int NormalTable::getBufferIndexForPrimaryKey(ValidItemID primaryKey) const
{
	int index = 0;
	for (auto iter = buffer.constBegin(); iter != buffer.constEnd(); iter++) {
		if ((*iter)->at(0) == primaryKey.get()) return index;
		index++;
	}
	return -1;
}

ValidItemID NormalTable::getPrimaryKeyAt(int bufferRowIndex) const
{
	return buffer.at(bufferRowIndex)->at(primaryKeyColumn->getIndex()).toInt();
}



// MODIFICATIONS (PASSTHROUGH)

int NormalTable::addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data)
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

void NormalTable::removeRow(QWidget* parent, const ValidItemID primaryKey)
{
	return Table::removeRow(parent, { primaryKeyColumn }, { primaryKey });
}



// QABSTRACTIMTEMMODEL IMPLEMENTATION

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
				roleData.setData(buffer.at(rowIndex)->at(primaryKeyColumn->getIndex()));
			} else {
				roleData.setData(-1);
			}
			continue;
		}
		const Column* column = getColumnByIndex(columnIndex);
		
		QVariant bufferValue = (rowIndex < 0) ? QVariant() : buffer.at(rowIndex)->at(columnIndex);
		QVariant result = QVariant();
		
		switch (column->type) {
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
