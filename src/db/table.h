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



#endif // TABLE_H
