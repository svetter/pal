#ifndef TABLE_H
#define TABLE_H

#include "column.h"

#include <QAbstractTableModel>
#include <QString>
#include <QWidget>



class Table : public QAbstractItemModel {
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
	QList<int> getMatchingBufferRowIndices(const Column* column, const QVariant& content) const;
	int getMatchingBufferRowIndex(const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys) const;
	void printBuffer() const;
	
	// Getters
	virtual int getNumberOfColumns() const = 0;
	virtual QList<const Column*> getColumnList() const = 0;
	virtual QList<const Column*> getPrimaryKeyColumnList() const = 0;
	QString getColumnListString() const;
	int getColumnIndex(const Column* column) const;
	const Column* getColumnByIndex(int index) const;
	
protected:
	// Modifications
	int addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys);
	
private:
	// SQL
	QList<QList<QVariant>*>* getAllEntriesFromSql(QWidget* parent) const;
	int addRowToSql(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRowFromSql(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<QVariant>& primaryKeys);
	
public:
	// QAbstractItemModel implementation
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	
	QModelIndex getNormalRootModelIndex() const;
};



#endif // TABLE_H
