/*
 * Copyright 2023-2024 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file table.h
 * 
 * This file declares the Table class.
 */

#ifndef TABLE_H
#define TABLE_H

#include "column.h"
#include "src/data/item_id.h"
#include "src/db/table_buffer.h"

#include <QAbstractTableModel>
#include <QString>
#include <QWidget>

using std::unique_ptr;

typedef QPair<const Column*, QVariant> ColumnDataPair;

class Database;



/**
 * A class for accessing and manipulating a table in the database.
 * 
 * This class is a QAbstractItemModel, so it can be used as a model for a QTableView.
 */
class Table : public QAbstractItemModel {
protected:
	Database& db;
	
private:
	/** The columns of this table. */
	QList<const Column*> columns;
	
	/** The row change listener which needs to be notified of row changes. */
	unique_ptr<const RowChangeListener> rowChangeListener;
	
public:
	/** The internal name of the table. */
	const QString	name;
	/** The name of the table as it should be displayed in the UI. */
	const QString	uiName;
	/** Whether the table is associative, i.e. its primary key is a combination of foreign keys. */
	const bool		isAssociative;
	
protected:
	/** The buffer for this table. */
	TableBuffer buffer;
	
	Table(Database& db, QString name, QString uiName, bool isAssociative);
public:
	virtual ~Table();
	
protected:
	void addColumn(const Column& column);
	
public:
	// Column info
	int getNumberOfColumns() const;
	int getNumberOfPrimaryKeyColumns() const;
	QList<const Column*> getColumnList() const;
	QList<const Column*> getPrimaryKeyColumnList() const;
	QList<const PrimaryKeyColumn*> getPrimaryKeyColumnTypedList() const;
	QList<const Column*> getForeignKeyColumnList() const;
	QList<const ForeignKeyColumn*> getForeignKeyColumnTypedList() const;
	QList<const Column*> getNonPrimaryKeyColumnList() const;
	QString getColumnListString() const;
	QString getPrimaryKeyColumnListString() const;
	int getColumnIndex(const Column& column) const;
	const Column& getColumnByIndex(int index) const;
	
	// Buffer access
	void initBuffer(QWidget& parent);
	void resetBuffer();
	int getNumberOfRows() const;
	const QList<QVariant>* getBufferRow(BufferRowIndex bufferRowIndex) const;
	QList<BufferRowIndex> getMatchingBufferRowIndices(const Column& column, const QVariant& content) const;
	BufferRowIndex getMatchingBufferRowIndex(const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys) const;
	// Debugging
	void printBuffer() const;
	
	// Change propagation
	void setRowChangeListener(unique_ptr<const RowChangeListener> newListener);
private:
	void notifyChangeListeners(const QSet<const Column*>& changedColumns);
	void notifyForAllColumns();
	
protected:
	// Modifications
	BufferRowIndex addRow(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs);
	void updateCellInNormalTable(QWidget& parent, const ValidItemID primaryKey, const Column& column, const QVariant& data);
	void updateRowsInNormalTable(QWidget& parent, const QSet<BufferRowIndex>& bufferIndices, const QList<ColumnDataPair>& columnDataPairs);
	void updateRowInNormalTable(QWidget& parent, const ValidItemID primaryKey, QList<ColumnDataPair>& columnDataPairs);
	void removeRow(QWidget& parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys);
	void removeMatchingRows(QWidget& parent, const Column& column, const QSet<ValidItemID>& keys);
	void removeMatchingRows(QWidget& parent, const Column& column, ValidItemID key);
	
private:
	// SQL
	void createTableInSql(QWidget& parent);
	QList<QList<QVariant>*> getAllEntriesFromSql(QWidget& parent) const;
	ValidItemID addRowToSql(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs);
	void updateCellOfNormalTableInSql(QWidget& parent, const ValidItemID primaryKey, const Column& column, const QVariant& data);
	void updateRowInSql(QWidget& parent, const ValidItemID primaryKey, const QList<ColumnDataPair>& columnDataPairs);
	void removeRowFromSql(QWidget& parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys);
	void removeMatchingRowsFromSql(QWidget& parent, const Column& column, ValidItemID key);
	QString getColumnListStringFrom(const QList<ColumnDataPair>& columnDataPairs);
	
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
	friend class DatabaseUpgrader;
	friend class ProjectSettings;
};



#endif // TABLE_H
