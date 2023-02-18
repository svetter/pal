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
	QList<QList<QVariant>*>* buffer;
	
	Table(QString name, QString uiName, bool isAssociative);
public:
	virtual ~Table();
	
	QString getName() const;
	QString getUIName() const;
	bool isAssociative() const;
	
	// Buffer
	void deleteBuffer();
	void initBuffer(QWidget* parent);
	const QList<QVariant>* getBufferRow(int rowIndex) const;
	
	// Getters
	virtual int getNumberOfColumns() const = 0;
	virtual QList<const Column*> getColumnList() const = 0;
	QString getColumnListString() const;
	int getColumnIndex(const Column* column) const;
	
protected:
	// Modifications
	int addRow(QWidget* parent, const QList<QVariant>& data, const QList<const Column*>& columns);
	WhatIfResult whatIf_removeRow(int primaryKey);
	template<typename T> void changeCell(int ascentID, const Column* column, T newValue);
	
private:
	// SQL
	QList<QList<QVariant>*>* getAllEntriesFromSql(QWidget* parent) const;
	int addRowToSql(QWidget* parent, const QList<QVariant>& data, const QList<const Column*>& columns);
};



#endif // TABLE_H
