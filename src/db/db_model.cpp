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


QString Column::getName()
{
	return name;
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

QString NormalTable::getColumnListString()
{
	assert(!nonPrimaryColumns.empty());
	return primaryKeyColumn->name + ", " + getNonPrimaryKeyColumnListString();
}

QString NormalTable::getNonPrimaryKeyColumnListString()
{
	QString result = "";
	bool first = true;
	for (auto iter = nonPrimaryColumns.begin(); iter != nonPrimaryColumns.end(); iter++) {
		result = result + (first ? "" : ", ") + (*iter)->name;
		first = false;
	}
	return result;
}

int NormalTable::getNumberOfNonPrimaryKeyColumns()
{
	return nonPrimaryColumns.size();
}

Column* NormalTable::getColumnByName(QString name)
{
	for (auto iter = nonPrimaryColumns.begin(); iter != nonPrimaryColumns.end(); iter++) {
		if ((*iter)->name == name) return *iter;
	}
	return nullptr;
}

int NormalTable::getColumnIndex(Column* column)
{
	if (column == getPrimaryKeyColumn()) {
		return 0;
	}
	int i = 1;
	for (auto iter = nonPrimaryColumns.constBegin(); iter != nonPrimaryColumns.constEnd(); iter++) {
		if (*iter == column) return i;
		i++;
	}
	return -1;
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
