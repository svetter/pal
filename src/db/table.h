#ifndef TABLE_H
#define TABLE_H

#include "column.h"

#include <QString>
#include <QWidget>



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
	const QList<QVariant>* getBufferRow(int rowIndex) const;
	
	// Getters
	virtual int getNumberOfColumns() const = 0;
	virtual QList<Column*> getColumnList() const = 0;
	QString getColumnListString() const;
	int getColumnIndex(Column* column) const;
	
protected:
	// Modifications
	int addRow(QWidget* parent, const QList<QVariant>& data, const QList<Column*>& columns);
	WhatIfResult whatIf_removeRow(int primaryKey);
	WhatIfResult whatIf_changeCell(int primaryKey, Column* column);
	template<typename T> void changeCell(int ascentID, Column* column, T newValue);
	
private:
	// SQL
	QList<QList<QVariant>*>* getAllEntriesFromSql(QWidget* parent) const;
	int addRowToSql(QWidget* parent, const QList<QVariant>& data, const QList<Column*>& columns);
};



#endif // TABLE_H
