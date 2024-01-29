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

#include "composite_column.h"
#include "src/db/row_index.h"
#include "src/db/database.h"

#include <QAbstractTableModel>
#include <QProgressDialog>
#include <QTableView>



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
 * The change annunciation consists of two parts:
 * First, the CompositeTable must be notified when a row is inserted or removed from the base table.
 * This is done by registering the table as a row change listener with the base table, which then
 * calls bufferRowJustInserted() or bufferRowAboutToBeRemoved() on the CompositeTable.
 * Second, the CompositeTable must be notified when data in any of the columns it depends on changes
 * (without adding or removing rows). This is done by registering each composite column as a change
 * listener with every column it depends on using Column::registerChangeListener().
 * Any changes in a base column then trigger a call to CompositeColumn::announceChangedData(), which
 * in turn calls announceChangesUnderColumn(), which either updates the column immediately or marks
 * it as dirty.
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
	
	/** The project database. */
	const Database* const db;
	/** The database table this table is based on. */
	const NormalTable* const baseTable;
	/** The UI table view this table is displayed in. */
	QTableView* const tableView;
	
	/** The composite columns of this table in their default order. */
	QList<const CompositeColumn*> columns;
	/** The index of the first column which is never shown in UI, but only used for filtering. */
	int firstFilterColumnIndex;
	/** The composite columns of this table which are only used for exporting, not for display in the UI. Paired with the index of the normal column they come in front of. */
	QList<QPair<int, const CompositeColumn*>> exportColumns;
	
	/** Whether the buffer has been initialized for an open project. */
	bool bufferInitialized;
	/** The buffer used for storing the raw computed values of the cells. */
	TableBuffer buffer;
	/** The order buffer used to change which rows are displayed in the UI and in which order. */
	ViewOrderBuffer viewOrder;
	/** The currently applied sorting, as a pair of the column to sort by and the sort order. */
	SortingPass currentSorting;
	/** The set of currently applied filters. */
	QSet<Filter> currentFilters;
	
	/** The current set of dirty columns which need to be updated before reading the buffer. */
	QSet<const CompositeColumn*> dirtyColumns;
	/** The set of columns which are currently hidden and therefore do not need to be updated unless they are used for sorting and/or filtering. */
	QSet<const CompositeColumn*> hiddenColumns;
	/** Whether the table is currently set to update its columns immediately when notified of changes in the database. */
	bool updateImmediately;
	/** A pointer to the UI table view which, if set, is automatically resized after the buffer is computed. */
	QTableView* tableToAutoResizeAfterCompute;
	
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
	
	CompositeTable(const Database* db, NormalTable* baseTable, QTableView* tableView);
public:
	~CompositeTable();
	
protected:
	void addColumn(const CompositeColumn* column);
	void addExportOnlyColumn(const CompositeColumn* column);
	void addFilterColumn(const CompositeColumn* column);
public:
	int getNumberOfNormalColumns() const;
	int getNumberOfColumnsForCompleteExport() const;
	QList<const CompositeColumn*> getNormalColumnList() const;
	QList<const CompositeColumn*> getCompleteExportColumnList() const;
	const CompositeColumn* getColumnAt(int columnIndex) const;
	const CompositeColumn* getExportOnlyColumnAt(int columnIndex) const;
	const CompositeColumn* getColumnByName(const QString& columnName) const;
	int getIndexOf(const CompositeColumn* column) const;
	int getExportIndexOf(const CompositeColumn* column) const;
	QSet<QString> getNormalColumnNameSet() const;
	const NormalTable* getBaseTable() const;
	
	int getNumberOfCellsToInit() const;
	void initBuffer(QProgressDialog* progressDialog, bool deferCompute = false, QTableView* tableToAutoResizeAfterCompute = nullptr);
	void rebuildOrderBuffer(bool skipRepopulate = false);
	QSet<const CompositeColumn*> getColumnsToUpdate() const;
	int getNumberOfCellsToUpdate() const;
	void updateBufferColumns(QSet<const CompositeColumn*> columnsToUpdate, std::function<void()> runAfterEachCellUpdate = []() {});
	void updateBothBuffers(std::function<void()> runAfterEachCellUpdate = []() {});
	void resetBuffer();
	BufferRowIndex getBufferRowIndexForViewRow(ViewRowIndex viewRowIndex) const;
	ViewRowIndex findViewRowIndexForBufferRow(BufferRowIndex bufferRowIndex) const;
	
	QVariant getRawValue(BufferRowIndex bufferRowIndex, const CompositeColumn* column);
	QVariant getFormattedValue(BufferRowIndex bufferRowIndex, const CompositeColumn* column);
	
	virtual SortingPass getDefaultSorting() const = 0;
	SortingPass getCurrentSorting() const;
	
	void setInitialFilters(QSet<Filter> filters);
	void applyFilters(QSet<Filter> filters);
	void clearFilters();
	QSet<Filter> getCurrentFilters() const;
	bool filterIsActive() const;
	
public:
	// Mark columns hidden or unhidden
	void markColumnHidden(int columnIndex);
	void markColumnUnhidden(int columnIndex);
	void markAllColumnsUnhidden();
	// Change annunciation
	void setUpdateImmediately(bool updateImmediately, QProgressDialog* progress = nullptr);
	void bufferRowJustInserted(BufferRowIndex bufferRowIndex);
	void bufferRowAboutToBeRemoved(BufferRowIndex bufferRowIndex);
	void announceChangesUnderColumn(int columnIndex);
	
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
	
protected:
	Breadcrumbs crumbsTo(const Database* db, const NormalTable* destinationTable) const;
	
public:
	ProjectSettings* getProjectSettings() const;
	
signals:
	/**
	 * Emitted after the table was resorted.
	 */
	void wasResorted();
};



#endif // COMPOSITE_TABLE_H
