#include "column.h"

#include "table.h"



WhatIfResult::WhatIfResult(const Table* affectedTable, QSet<int> affectedIDs) :
		affectedTable(affectedTable),
		affectedIDs(affectedIDs)
{}



Column::Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, const Column* foreignKey, const Table* inTable) :
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


QString Column::getName() const
{
	return name;
}

QString Column::getUIName() const
{
	return uiName;
}

DataType Column::getType() const
{
	return type;
}

bool Column::isPrimaryKey() const
{
	return primaryKey;
}

bool Column::isForeignKey() const
{
	return foreignKey;
}

const Column* Column::getReferencedForeignColumn() const
{
	return foreignKey;
}

bool Column::isNullable() const
{
	return nullable;
}

const Table* Column::getTable() const
{
	return inTable;
}


int Column::getIndex() const
{
	return inTable->getColumnIndex(this);
}



QString getColumnListStringOf(QList<const Column*> columns)
{
	QString result = "";
	bool first = true;
	for (auto iter = columns.begin(); iter != columns.end(); iter++) {
		result = result + (first ? "" : ", ") + (*iter)->getName();
		first = false;
	}
	return result;
}
