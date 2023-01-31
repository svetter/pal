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
	Column		* foreignKey;
	bool		nullable;
	Table		* inTable;
	
public:
	Column(QString name, QString uiName, ColumnType type, bool nullable, bool primaryKey, Column * foreignKey, Table * inTable);
	
	friend class NormalTable;
	friend class AssociativeTable;
};


class Table {
	QString	name;
	QString	uiName;
	
protected:
	Table(QString name,  QString uiName);
};


class NormalTable: Table {
	Column			primaryKeyColumn;
	QList<Column>	nonPrimaryColumns;
	
public:
	NormalTable(QString name, QString uiName);
	
	void addColumn(QString name, QString uiName, ColumnType type, bool nullable = true, Column * foreignKey = nullptr);
	
	const Column * getPrimaryKeyColumn() const;
	const Column * getColumnByName(QString name) const;
};


class AssociativeTable: Table {
	Column	column1;
	Column	column2;
	
public:
	AssociativeTable(QString name, QString uiName, Column *foreignKeyColumn1, Column * foreignKeyColumn2);
	
	const Column * getColumn1() const;	
	const Column * getColumn2() const;
};



#endif // DB_MODEL_H
