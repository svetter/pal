#include "associative_table.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QModelIndex>



AssociativeTable::AssociativeTable(QString name, Column* foreignKeyColumn1, Column* foreignKeyColumn2) :
		Table(name, QString(), true),
		column1(new Column(foreignKeyColumn1->name, foreignKeyColumn1->uiName, DataType::integer, false, true, foreignKeyColumn1, this)),
		column2(new Column(foreignKeyColumn2->name, foreignKeyColumn2->uiName, DataType::integer, false, true, foreignKeyColumn2, this))
{
	assert(foreignKeyColumn1->primaryKey && foreignKeyColumn1->type == DataType::integer);
	assert(foreignKeyColumn2->primaryKey && foreignKeyColumn2->type == DataType::integer);
	
	addColumn(column1);
	addColumn(column2);
}

AssociativeTable::~AssociativeTable()
{}



// COLUMN INFO

Column* AssociativeTable::getColumn1() const
{
	return column1;
}

Column* AssociativeTable::getColumn2() const
{
	return column2;
}

const Column* AssociativeTable::getOtherColumn(const Column* column) const
{
	if (column == column1) return column2;
	if (column == column2) return column1;
	return nullptr;
}

const Column* AssociativeTable::getOwnColumnReferencing(const Column* foreignColumn) const
{
	if (column1->foreignKey == foreignColumn)	return column1;
	if (column2->foreignKey == foreignColumn)	return column2;
	return nullptr;
}

const NormalTable* AssociativeTable::traverseAssociativeRelation(const Column* foreignColumn) const
{
	const Column* matchingOwnColumn = getOwnColumnReferencing(foreignColumn);
	if (!matchingOwnColumn) return nullptr;
	
	const Column* otherColumn = getOtherColumn(matchingOwnColumn);
	return (NormalTable*) otherColumn->foreignKey->table;
}



// BUFFER ACCESS

int AssociativeTable::getNumberOfMatchingRows(const Column* column, ValidItemID primaryKey) const
{
	assert(column == column1 || column == column2);
	int numberOfMatches = 0;
	for (auto iter = buffer.constBegin(); iter != buffer.constEnd(); iter++) {
		if ((*iter)->at(column->getIndex()) == primaryKey.get()) {
			numberOfMatches++;
		}
	}
	return numberOfMatches;
}

QSet<ValidItemID> AssociativeTable::getMatchingEntries(const Column* column, ValidItemID primaryKey) const
{
	assert(column == column1 || column == column2);
	const Column* otherColumn = getOtherColumn(column);
	QSet<ValidItemID> filtered = QSet<ValidItemID>();
	for (auto iter = buffer.constBegin(); iter != buffer.constEnd(); iter++) {
		if ((*iter)->at(column->getIndex()) == primaryKey.get()) {
			filtered.insert((*iter)->at(otherColumn->getIndex()).toInt());
		}
	}
	return filtered;
}



// MODIFICATIONS (PASSTHROUGH)

void AssociativeTable::addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data)
{
	Table::addRow(parent, columns, data);
}

void AssociativeTable::removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys)
{
	return Table::removeRow(parent, primaryKeyColumns, primaryKeys);
}

void AssociativeTable::removeMatchingRows(QWidget* parent, const Column* column, ValidItemID primaryKey)
{
	return Table::removeMatchingRows(parent, column, primaryKey);
}



// QABSTRACTIMTEMMODEL IMPLEMENTATION

void AssociativeTable::multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const
{
	assert(getColumnByIndex(index.column())->type == integer);
	
	for (QModelRoleData& roleData : roleDataSpan) {
		int role = roleData.role();
		if (role == Qt::TextAlignmentRole) {
			roleData.setData(Qt::AlignRight);
			continue;
		}
		if (role == Qt::DisplayRole) {
			QVariant result = buffer.at(index.row())->at(index.column());
			roleData.setData(result);
			continue;
		}
	}
}
