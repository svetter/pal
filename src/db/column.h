#ifndef COLUMN_H
#define COLUMN_H

#include <QList>



enum DataType {
	integer, bit, varchar, date, time_
};


class Table;
class NormalTable;
class AssociativeTable;


struct WhatIfResult {
	QList<Table*>	affectedTables;
	int				numAffectedRows;
};



class Column {
	QString		name;
	QString		uiName;
	DataType	type;
	bool		primaryKey;
	Column*		foreignKey;
	bool		nullable;
	Table*		inTable;
	
public:
	Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, Column* foreignKey, Table* inTable);
	
	QString getName();
	QString getUIName();
	DataType getType();
	bool isPrimaryKey();
	bool isForeignKey();
	Column* getReferencedForeignColumn();
	bool isNullable();
	Table* getTable();
	
	int getIndex();
	
	friend class NormalTable;
	friend class AssociativeTable;
};



QString getColumnListStringOf(QList<Column*> columns);



#endif // COLUMN_H
