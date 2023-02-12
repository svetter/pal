#include "associative_table.h"



AssociativeTable::AssociativeTable(QString name, Column* foreignKeyColumn1, Column* foreignKeyColumn2) :
		Table(name, QString(), true),
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

int AssociativeTable::getNumberOfColumns() const
{
	return 2;
}

QList<Column*> AssociativeTable::getColumnList() const
{
	return { column1, column2 };
}