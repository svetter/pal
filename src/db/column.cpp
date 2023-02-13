#include "column.h"

#include "table.h"



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



QString getColumnListStringOf(QList<Column*> columns)
{
	QString result = "";
	bool first = true;
	for (auto iter = columns.begin(); iter != columns.end(); iter++) {
		result = result + (first ? "" : ", ") + (*iter)->getName();
		first = false;
	}
	return result;
}
