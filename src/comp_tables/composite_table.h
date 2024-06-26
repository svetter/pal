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
 * @file composite_table.h
 * 
 * This file declares the CompositeTable class.
 */

#ifndef COMPOSITE_TABLE_H
#define COMPOSITE_TABLE_H

#include "src/comp_tables/comp_table_listener.h"
#include "src/comp_tables/composite_column.h"

#include <QTableView>
#include <QProgressDialog>

class Filter;



/**
 * A struct representing a single pass of sorting a composite table by a single column and sort
 * order.
 */
struct SortingPass {
	const CompositeColumn* column;
	Qt::SortOrder order;
};



/**
 * A class for UI-facing tables whose contents are compiled from potentially multiple different
 * columns of base tables in the database.
 * 
 * The content of each cell must be computed from whatever its sources in the database are (which
 * depends on the kind of CompositeColumn it is). A buffer is used to store all computed values.
 * However, the buffered values are not necessarily in their final form in which they are to be
 * displayed in the UI. The buffer stores "raw" computed values, which have to be formatted before
 * being shown in the UI, which is done on the fly in CompositeColumn::toFormattedTableContent().
 * 
 * To make sure the buffer stays up to date, the CompositeTable must be notified of any changes in
 * the underlying data in the database. For this purpose, there is a change annunciation mechanism.
 * Depending on a user setting, the table can either be updated immediately or only once it is
 * actually needed. In the latter case, the affected columns are marked as dirty and their update
 * deferred.
 * 
 * The CompositeTable is also responsible for sorting and filtering its content. For controlling
 * which rows are actually displayed and in which order, a ViewOrderBuffer is used. This buffer is
 * updated whenever the sorting or filtering changes. The buffer is then used to map the indices of
 * the rows in the buffer (BufferRowIndex) to the indices of the rows in the UI (ViewRowIndex). It
 * is important that these two different kinds of indices are not conflated.
 * 
 * This class implements the QAbstractTableModel interface so that it can be used as a model for a
 * QTableView.
 * 
 * @see CompositeColumn
 */
class CompositeTable : public QAbstractTableModel {
	Q_OBJECT
	
public:
	/** The project database. */
	Database& db;
	/** The database table this table is based on. */
	const NormalTable& baseTable;
private:
	/** The UI table view this table is displayed in. */
	QTableView* const tableView;
	
	/** The composite columns of this table in their default order. */
	QList<const CompositeColumn*> columns;
	/** The composite columns of this table which are only used for exporting, not for display in the UI. Paired with the index of the default column they come in front of. */
	QList<QPair<int, const CompositeColumn*>> exportColumns;
	/** The custom composite columns of this table created by the user in order of creation. */
	QList<const CompositeColumn*> customColumns;
	/** The names of all static (default and export) columns. */
	QStringList staticColumnNames;
	/** The names of the current custom columns. */
	QStringList customColumnNames;
	
	/** Whether the buffer has been initialized for an open project. */
	bool bufferInitialized;
	/** The buffer used for storing the raw computed values of the cells. */
	TableBuffer buffer;
	/** The order buffer used to change which rows are displayed in the UI and in which order. */
	ViewOrderBuffer viewOrder;
	/** The currently applied sorting, as a pair of the column to sort by and the sort order. */
	SortingPass currentSorting;
	/** The set of currently applied filters. */
	QList<const Filter*> currentFilters;
	
	/** The current set of dirty columns which need to be updated before reading the buffer. */
	QSet<const CompositeColumn*> dirtyColumns;
	/** The set of columns which are currently hidden and therefore do not need to be updated unless they are used for sorting and/or filtering. */
	QSet<const CompositeColumn*> hiddenColumns;
	/** Whether the table is currently set to update its columns immediately when notified of changes in the database. */
	bool updateImmediately;
	/** A pointer to the UI table view which, if set, is automatically resized after the buffer is computed. */
	QTableView* tableToAutoResizeAfterCompute;
	
	/** The change listener for all changes under this composite table. */
	TableChangeListenerCompositeTable changeListener;
	
public:
	/** The internal name of the table (not for display in the UI). */
	const QString name;
	/** The name of the table as it should be displayed in the UI. */
	const QString uiName;
	
protected:
	/** An empty string to use as suffix for columns which don't need one. */
	static inline QString noSuffix = QString();
	/** The suffix to append to all values given in meters. */
	static inline QString mSuffix = " m";
	
	CompositeTable(Database& db, NormalTable& baseTable, QTableView* tableView);
public:
	~CompositeTable();
	
	void reset();
	
protected:
	void addColumn(const CompositeColumn& newColumn);
	void addExportOnlyColumn(const CompositeColumn& newColumn);
public:
	void addCustomColumn(const CompositeColumn& newColumn);
	void setCustomColumns(const QList<CompositeColumn*> newCustomColumns);
	void removeCustomColumnAt(int logicalIndex);
	
	int getNumberOfNormalColumns() const;
	int getNumberOfColumnsForCompleteExport() const;
	QList<const CompositeColumn*> getNormalColumnList() const;
	QList<const CompositeColumn*> getCompleteExportColumnList() const;
	const CompositeColumn& getColumnAt(int columnIndex) const;
	const CompositeColumn& getExportOnlyColumnAt(int columnIndex) const;
	const CompositeColumn* getColumnByNameOrNull(const QString& columnName) const;
	bool hasCustomColumnAt(int logicalIndex) const;
	QString getEncodedCustomColumns() const;
	int getIndexOf(const CompositeColumn& column) const;
	int getExportIndexOf(const CompositeColumn& column) const;
	QSet<QString> getNormalColumnNameSet() const;
	
	int getNumberOfCellsToInit() const;
	void initBuffer(QProgressDialog* progressDialog, bool deferCompute = false, QTableView* tableToAutoResizeAfterCompute = nullptr);
	void rebuildOrderBuffer(bool skipRepopulate = false);
	QSet<const CompositeColumn*> getColumnsToUpdate() const;
	int getNumberOfCellsToUpdate() const;
	void updateBufferColumns(QSet<const CompositeColumn*> columnsToUpdate, std::function<void()> runAfterEachCellUpdate = []() {});
	void updateBothBuffers(std::function<void()> runAfterEachCellUpdate = []() {});
	BufferRowIndex getBufferRowIndexForViewRow(ViewRowIndex viewRowIndex) const;
	ViewRowIndex findViewRowIndexForBufferRow(BufferRowIndex bufferRowIndex) const;
	
	QVariant getRawValue(BufferRowIndex bufferRowIndex, const CompositeColumn& column);
	QVariant getFormattedValue(BufferRowIndex bufferRowIndex, const CompositeColumn& column);
	
	virtual SortingPass getDefaultSorting() const = 0;
	SortingPass getCurrentSorting() const;
	
	// Filters
	void setInitialFilters(const QList<const Filter*>& filters);
	void applyFilters(const QList<const Filter*>& filters);
	void clearFilters();
	QList<const Filter*> getCurrentFilters() const;
	bool filterIsActive() const;
	
public:
	// Mark columns hidden or unhidden
	void markColumnHidden(int columnIndex);
	void markColumnUnhidden(int columnIndex);
	void markAllColumnsUnhidden();
	bool isColumnHidden(const CompositeColumn& column) const;
	// Change annunciation
	void setUpdateImmediately(bool updateImmediately, QProgressDialog* progress = nullptr);
	void announceChanges(const QSet<const Column*>& affectedColumns, const QList<QPair<BufferRowIndex, bool>>& rowsAddedOrRemoved);
	
	// QAbstractTableModel implementation
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	void sort(int columnIndex, Qt::SortOrder order = Qt::AscendingOrder) override;
private:
	void performSort(SortingPass previousSort, bool allowPassAndReverse);
	
	QVariant computeCellContent(BufferRowIndex bufferRowIndex, int columnIndex) const;
	QList<QVariant> computeWholeColumnContent(int columnIndex) const;
	
public:
	Breadcrumbs crumbsTo(const NormalTable& destinationTable) const;
	
	const ProjectSettings& getProjectSettings() const;
	
signals:
	/**
	 * Emitted after the table was resorted.
	 */
	void wasResorted();
};



#endif // COMPOSITE_TABLE_H
