#ifndef DB_MODEL_H
#define DB_MODEL_H

#include <QString>
#include <QList>
#include <QDate>
#include <QTime>



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
	
	friend class NormalTable;
	friend class AssociativeTable;
};


class Table {
	QString	name;
	QString	uiName;
	
protected:
	Table(QString name,  QString uiName);
	
public:
	WhatIfResult whatIf_removeRow(int primaryKey);
	void removeRow(int primaryKey);
	WhatIfResult whatIf_changeCell(int primaryKey, Column* column);
	template<typename T> void changeCell(int ascentID, Column* column, T newValue);
};


class NormalTable: Table {
	Column			primaryKeyColumn;
	QList<Column>	nonPrimaryColumns;
	
public:
	NormalTable(QString name, QString uiName);
	
	void addColumn(QString name, QString uiName, DataType type, bool nullable = true, Column* foreignKey = nullptr);
	
	const Column* getPrimaryKeyColumn() const;
	const Column* getColumnByName(QString name) const;
};


class AssociativeTable: Table {
	Column	column1;
	Column	column2;
	
public:
	AssociativeTable(QString name, QString uiName, Column* foreignKeyColumn1, Column* foreignKeyColumn2);
	
	const Column* getColumn1() const;	
	const Column* getColumn2() const;
};



#endif // DB_MODEL_H
