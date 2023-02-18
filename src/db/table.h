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
	QSet<int> getMatchingBufferRowIndices(const Column* column, const QVariant& content) const;
	
	// Getters
	virtual int getNumberOfColumns() const = 0;
	virtual QList<const Column*> getColumnList() const = 0;
	virtual QList<const Column*> getPrimaryKeyColumnList() const = 0;
	QString getColumnListString() const;
	int getColumnIndex(const Column* column) const;
	
protected:
	// Modifications
	int addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys);
	
private:
	// SQL
	QList<QList<QVariant>*>* getAllEntriesFromSql(QWidget* parent) const;
	int addRowToSql(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	int removeRowFromSql(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys);
	
	int getMatchingBufferRowIndex(const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys) const;
};



#endif // TABLE_H
