#include "associative_table.h"

#include "src/db/db_error.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QModelIndex>



AssociativeTable::AssociativeTable(QString name, const Column* foreignKeyColumn1, const Column* foreignKeyColumn2) :
		Table(name, QString(), true),
		column1(new const Column(foreignKeyColumn1->name, foreignKeyColumn1->uiName, DataType::integer, false, true, foreignKeyColumn1, this)),
		column2(new const Column(foreignKeyColumn2->name, foreignKeyColumn2->uiName, DataType::integer, false, true, foreignKeyColumn2, this))
{
	assert(foreignKeyColumn1->primaryKey && foreignKeyColumn1->type == DataType::integer);
	assert(foreignKeyColumn2->primaryKey && foreignKeyColumn2->type == DataType::integer);
}

AssociativeTable::~AssociativeTable()
{
	delete column1;
	delete column2;
}


const Column* AssociativeTable::getColumn1() const
{
	return column1;
}

const Column* AssociativeTable::getColumn2() const
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
	return (NormalTable*) otherColumn->foreignKey->inTable;
}


int AssociativeTable::getNumberOfColumns() const
{
	return 2;
}

QList<const Column*> AssociativeTable::getColumnList() const
{
	return { column1, column2 };
}

QList<const Column*> AssociativeTable::getPrimaryKeyColumnList() const
{
	return getColumnList();
}



QSet<ValidItemID> AssociativeTable::getMatchingEntries(const Column* column, ValidItemID primaryKey) const
{
	assert(column == column1 || column == column2);
	const Column* otherColumn = getOtherColumn(column);
	QSet<ValidItemID> filtered = QSet<ValidItemID>();
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(column->getIndex()) == primaryKey.get()) {
			filtered.insert((*iter)->at(otherColumn->getIndex()).toInt());
		}
	}
	return filtered;
}

int AssociativeTable::getNumberOfMatchingRows(const Column* column, ValidItemID primaryKey) const
{
	assert(column == column1 || column == column2);
	int numberOfMatches = 0;
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(column->getIndex()) == primaryKey.get()) {
			numberOfMatches++;
		}
	}
	return numberOfMatches;
}



void AssociativeTable::addRow(QWidget* parent, const QList<QVariant>& data)
{
	assert(data.size() == 2);
	
	Table::addRow(parent, getColumnList(), data);
}

void AssociativeTable::removeRow(QWidget* parent, const QList<ValidItemID>& primaryKeys)
{
	assert(primaryKeys.size() == 2);
	
	// TODO #71
	qDebug() << "UNIMPLEMENTED: AssociativeTable::removeRow()";
}

void AssociativeTable::removeMatchingRows(QWidget* parent, const Column* column, ValidItemID primaryKey)
{
	Table::removeMatchingRows(parent, column, primaryKey);
}
