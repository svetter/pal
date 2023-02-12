#ifndef DB_MODEL_H
#define DB_MODEL_H

#include <QString>
#include <QList>
#include <QDate>
#include <QTime>
#include <QAbstractTableModel>



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



QString getColumnListString(QList<Column*> columns);



class Table {
	QString	name;
	QString	uiName;
	bool associative;
	
protected:	
	Table(QString name, QString uiName, bool isAssociative);
	~Table();
	
	QList<QList<QVariant>*>* buffer;
	
public:
	QString getName();
	QString getUIName();
	bool isAssociative();
	
	// Buffer
	void deleteBuffer();
	void initBuffer(QWidget* parent);
	void ensureBuffer(QWidget* parent);
	
	// Getters
	virtual int getNumberOfColumns() const = 0;
	virtual QList<Column*> getColumnList() const = 0;
	int getColumnIndex(Column* column) const;
	QString getColumnListString() const;
	QList<QList<QVariant>*>* getAllEntries(QWidget* parent) const;
	
	// Modification
	WhatIfResult whatIf_removeRow(int primaryKey);
	void removeRow(int primaryKey);
	WhatIfResult whatIf_changeCell(int primaryKey, Column* column);
	template<typename T> void changeCell(int ascentID, Column* column, T newValue);
};



class NormalTable : public Table, public QAbstractItemModel {
	QString noneString;
	
	Column*			primaryKeyColumn;
	QList<Column*>	nonPrimaryColumns;
	
public:
	NormalTable(QString name, QString itemNameSingularLowercase, QString uiName, QString noneString);
	~NormalTable();
	
	void addColumn(Column* column);
	
	// Getters
	QList<Column*> getColumnList() const override;
	int getNumberOfColumns() const override;
	Column* getPrimaryKeyColumn() const;
	int getNumberOfNonPrimaryKeyColumns() const;
	QList<Column*> getNonPrimaryKeyColumnList() const;
	Column* getColumnByIndex(int index) const;
	int getBufferIndexForPrimaryKey(int primaryKey) const;
	
	int getNumberOfEntries(QWidget* parent);
	
	// Modifications
	int addRow(QList<QVariant>& data);
	
	// QAbstractItemModel implementation
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex getNormalRootModelIndex() const;
	QModelIndex getNullableRootModelIndex() const;
};



class AssociativeTable : public Table {
protected:
	Column*	column1;
	Column*	column2;
	
public:
	AssociativeTable(QString name, Column* foreignKeyColumn1, Column* foreignKeyColumn2);
	~AssociativeTable();
	
	Column* getColumn1();
	Column* getColumn2();
	
	int getNumberOfColumns() const override;
	QList<Column*> getColumnList() const override;
};



#endif // DB_MODEL_H
