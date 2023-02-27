#ifndef TABLE_H
#define TABLE_H

#include "column.h"
#include "src/data/item_id.h"

#include <QAbstractTableModel>
#include <QString>
#include <QWidget>



class Table : public QAbstractItemModel {
	QList<const Column*> columns;

public:
	const QString	name;
	const QString	uiName;
	const bool		isAssociative;
	
protected:
	QList<QList<QVariant>*> buffer;
	
	Table(QString name, QString uiName, bool isAssociative);
public:
	virtual ~Table();
	
protected:
	void addColumn(const Column* column);
	
public:
	// Column info
	int getNumberOfColumns() const;
	int getNumberOfPrimaryKeyColumns() const;
	int getNumberOfNonPrimaryKeyColumns() const;
	QList<const Column*> getColumnList() const;
	QList<const Column*> getPrimaryKeyColumnList() const;
	QList<const Column*> getNonPrimaryKeyColumnList() const;
	QString getColumnListString() const;
	QString getPrimaryKeyColumnListString() const;
	int getColumnIndex(const Column* column) const;
	const Column* getColumnByIndex(int index) const;

	// Buffer access
	void initBuffer(QWidget* parent, bool expectEmpty = false);
	void resetBuffer();
	int getNumberOfRows() const;
	const QList<QVariant>* getBufferRow(int rowIndex) const;
	QList<int> getMatchingBufferRowIndices(const Column* column, const QVariant& content) const;
	int getMatchingBufferRowIndex(const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys) const;
	// Debugging
	void printBuffer() const;
	
protected:
	// Modifications
	int addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void updateCellInNormalTable(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data);
	void updateRowInNormalTable(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys);
	void removeMatchingRows(QWidget* parent, const Column* column, ValidItemID key);
	
private:
	// SQL
	void createTableInSql(QWidget* parent);
	QList<QList<QVariant>*> getAllEntriesFromSql(QWidget* parent, bool expectEmpty = false) const;
	int addRowToSql(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void updateCellInSql(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data);
	void updateRowInSql(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRowFromSql(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys);
	void removeMatchingRowsFromSql(QWidget* parent, const Column* column, ValidItemID key);
	
public:
	// QAbstractItemModel implementation (multiData implemented in subclasses)
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	
	static const int PrimaryKeyRole;
	QModelIndex getNormalRootModelIndex() const;
	QModelIndex getNullableRootModelIndex() const;
	
	friend class Database;
	friend class ProjectSettings;
};



#endif // TABLE_H
