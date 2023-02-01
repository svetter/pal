#include "db_model.h"

#include <assert.h>



Column::Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, Column * foreignKey, Table * inTable) :
		name(name),
		uiName(uiName),
		type(type),
		primaryKey(primaryKey),
		foreignKey(foreignKey),
		nullable(nullable && !primaryKey),
		inTable(inTable)
{}



Table::Table(QString name, QString uiName) :
		name(name),
		uiName(uiName)
{}



NormalTable::NormalTable(QString name, QString uiName) :
		Table(name, uiName),
		primaryKeyColumn(Column(name + "ID", "ID for table " + uiName, DataType::integer, false, true, nullptr, this)),
		nonPrimaryColumns(QList<Column>())
{}


void NormalTable::addColumn(QString name, QString uiName, DataType type, bool nullable, Column * foreignKey)
{
	nonPrimaryColumns.append(Column(name, uiName, type, nullable, false, foreignKey, this));
}


const Column * NormalTable::getPrimaryKeyColumn() const
{
	return &primaryKeyColumn;
}

const Column * NormalTable::getColumnByName(QString name) const
{
	QList<Column>::const_iterator iter = nonPrimaryColumns.begin();
	while (iter != nonPrimaryColumns.end()) {
		if (iter->name == name) return iter;
	}
	return nullptr;
}



AssociativeTable::AssociativeTable(QString name, QString uiName, Column * foreignKeyColumn1, Column * foreignKeyColumn2) :
		Table(name, uiName),
		column1(Column(foreignKeyColumn1->name, foreignKeyColumn1->uiName, DataType::integer, false, true, foreignKeyColumn1, this)),
		column2(Column(foreignKeyColumn2->name, foreignKeyColumn2->uiName, DataType::integer, false, true, foreignKeyColumn2, this))
{
	assert(foreignKeyColumn1->primaryKey && foreignKeyColumn1->type == DataType::integer);
	assert(foreignKeyColumn2->primaryKey && foreignKeyColumn2->type == DataType::integer);
}

const Column * AssociativeTable::getColumn1() const
{
	return &column1;
}

const Column * AssociativeTable::getColumn2() const
{
	return &column2;
}
