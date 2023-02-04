#include "db_model.h"

#include <assert.h>



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



Table::Table(QString name, QString uiName, bool isAssociative) :
		name(name),
		uiName(uiName),
		associative(isAssociative)
{}


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



NormalTable::NormalTable(QString name, QString itemNameSingularLowercase, QString uiName) :
		Table(name, uiName, false),
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


Column* NormalTable::getPrimaryKeyColumn()
{
	return primaryKeyColumn;
}

Column* NormalTable::getColumnByName(QString name)
{
	QList<Column*>::iterator iter = nonPrimaryColumns.begin();
	while (iter != nonPrimaryColumns.end()) {
		if ((*iter)->name == name) return *iter;
	}
	return nullptr;
}



AssociativeTable::AssociativeTable(QString name, QString uiName, Column* foreignKeyColumn1, Column* foreignKeyColumn2) :
		Table(name, uiName, true),
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
