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
	
	QString getName();
	
	friend class NormalTable;
	friend class AssociativeTable;
};



class Table {
	QString	name;
	QString	uiName;
	
protected:	
	bool associative;
	
	Table(QString name, QString uiName, bool isAssociative);
	
public:
	QString getName();
	QString getUIName();
	bool isAssociative();
	
	WhatIfResult whatIf_removeRow(int primaryKey);
	void removeRow(int primaryKey);
	WhatIfResult whatIf_changeCell(int primaryKey, Column* column);
	template<typename T> void changeCell(int ascentID, Column* column, T newValue);
};



class NormalTable : public Table {
	Column*			primaryKeyColumn;
	QList<Column*>	nonPrimaryColumns;
	
public:
	NormalTable(QString name, QString itemNameSingularLowercase, QString uiName);
	~NormalTable();
	
	void addColumn(Column* column);
	
	Column* getPrimaryKeyColumn();
	QString getColumnListString();
	Column* getColumnByName(QString name);
	int getColumnIndex(Column* column);
};



class AssociativeTable : public Table {
	Column*	column1;
	Column*	column2;
	
public:
	AssociativeTable(QString name, QString uiName, Column* foreignKeyColumn1, Column* foreignKeyColumn2);
	~AssociativeTable();
	
	Column* getColumn1();	
	Column* getColumn2();
};



#endif // DB_MODEL_H
