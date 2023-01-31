#ifndef DB_MODEL_H
#define DB_MODEL_H

#include <QString>
#include <QList>



enum ColumnType {
	integer, bit, varchar
};


class Table;
class NormalTable;
class AssociativeTable;


class Column {
	QString		name;
	QString		uiName;
	ColumnType	type;
	bool		primaryKey;
	Column		*foreignKey;
	bool		nullable;
	NormalTable	&inTable;
	
public:
	Column(QString name, QString uiName, ColumnType type, bool nullable, bool primaryKey, Column *foreignKey, Table &inTable);
	~Column();
	
	friend class Table;
};


class Table {
	QString	name;
	QString	uiName;
};


class NormalTable: Table {
	QList<Column>	columns;
	
public:
	NormalTable(QString name, QString uiName);
	~NormalTable();
	
	bool addColumn(QString name, QString uiName, ColumnType type, bool nullable = true, Column *foreignKey = nullptr);
	
	Column* getPrimaryKeyColumn() const;
};


class AssociativeTable: Table {
	Column	column1;
	Column	column2;
	
public:
	AssociativeTable(QString name, QString uiName, Column *foreignKeyColumn1, Column *foreignKeyColumn2);
	~AssociativeTable();
	
	QPair<Column*, Column*> getPrimaryKeyColumns() const;
};



#endif // DB_MODEL_H
