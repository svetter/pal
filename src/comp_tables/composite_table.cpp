/*
 * Copyright 2023 Simon Vetter
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
 * @file composite_table.cpp
 * 
 * This file defines the CompositeTable class.
 */

#include "composite_table.h"



/**
 * Creates a new CompositeTable.
 * 
 * @param db		The project database.
 * @param baseTable	The database table this table is based on.
 * @param tableView	The view this table is displayed in.
 */
CompositeTable::CompositeTable(const Database* db, NormalTable* baseTable, QTableView* tableView) :
	QAbstractTableModel(),
	db(db),
	baseTable(baseTable),
	tableView(tableView),
	columns(QList<const CompositeColumn*>()),
	firstFilterColumnIndex(-1),
	exportColumns(QList<QPair<int, const CompositeColumn*>>()),
	bufferInitialized(false),
	buffer(TableBuffer()),
	viewOrder(ViewOrderBuffer()),
	currentSorting({nullptr, Qt::AscendingOrder}),
	currentFilters(QSet<Filter>()),
	dirtyColumns(QSet<const CompositeColumn*>()),
	hiddenColumns(QSet<const CompositeColumn*>()),
	updateImmediately(false),
	tableToAutoResizeAfterCompute(nullptr),
	name(baseTable->name),
	uiName(baseTable->uiName)
{
	baseTable->setRowChangeListener(this);
}

/**
 * Destroys this CompositeTable.
 */
CompositeTable::~CompositeTable()
{
	qDeleteAll(columns);
	qDeleteAll(buffer);
}



/**
 * Adds a normal column to this table during initialization.
 * 
 * @param column	The composite column to add.
 */
void CompositeTable::addColumn(const CompositeColumn* column)
{
	assert(firstFilterColumnIndex < 0);
	
	columns.append(column);
	
	// Register as change listener at all underlying columns
	const QSet<Column* const> underlyingColumns = column->getAllUnderlyingColumns();
	for (Column* underlyingColumn : underlyingColumns) {
		underlyingColumn->registerChangeListener(column);
	}
}

/**
 * Adds an export-only column to this table during initialization.
 *
 * @param column	The composite column to add as an export-only column.
 */
void CompositeTable::addExportOnlyColumn(const CompositeColumn* column)
{
	assert(firstFilterColumnIndex < 0);
	
	exportColumns.append({columns.size(), column});
}

/**
 * Adds a filter-only column to this table during initialization.
 * 
 * @pre All normal and export-only columns have been added already.
 *
 * @param column	The composite column to add as a filter-only column.
 */
void CompositeTable::addFilterColumn(const CompositeColumn* column)
{
	if (firstFilterColumnIndex < 0) firstFilterColumnIndex = columns.size();
	
	columns.append(column);
	
	// Register as change listener at all underlying columns
	const QSet<Column* const> underlyingColumns = column->getAllUnderlyingColumns();
	for (Column* underlyingColumn : underlyingColumns) {
		underlyingColumn->registerChangeListener(column);
	}
}


/**
 * Returns the number of normal columns (for display in the UI) in this table.
 * 
 * @return	The number of visible columns in this table.
 */
int CompositeTable::getNumberOfNormalColumns() const
{
	if (firstFilterColumnIndex >= 0) return firstFilterColumnIndex;
	return columns.size();
}

/**
 * Returns the number of columns which can be exported from this table, including normal columns
 * and export-only columns.
 * 
 * @return	The number of columns which can be exported from this table.
 */
int CompositeTable::getNumberOfColumnsForCompleteExport() const
{
	return getNumberOfNormalColumns() + exportColumns.size();
}

/**
 * Returns a list of all normal (not export-only or filter-only) composite columns in the table.
 * 
 * @return	A list of all normal columns in the table.
 */
QList<const CompositeColumn*> CompositeTable::getNormalColumnList() const
{
	QList<const CompositeColumn*> list = QList<const CompositeColumn*>();
	for (int columnIndex = 0; columnIndex < getNumberOfNormalColumns(); columnIndex++) {
		list.append(columns.at(columnIndex));
	}
	return list;
}

/**
 * Returns a list of all composite columns which can be exported from this table, including normal
 * columns and export-only columns.
 * 
 * Sorts the export-only columns between the normal columns according to their assciated indices.
 * 
 * @return	A list of all columns which can be exported from this table.
 */
QList<const CompositeColumn*> CompositeTable::getCompleteExportColumnList() const
{
	QList<const CompositeColumn*> list = QList<const CompositeColumn*>();
	int numNormalColumns = getNumberOfNormalColumns();
	int exportOnlyColumnIndex = 0;
	int normalColumnIndex = 0;
	while (normalColumnIndex < numNormalColumns || exportOnlyColumnIndex < exportColumns.size()) {
		if (exportOnlyColumnIndex < exportColumns.size()) {
			// At least one export only column left
			// These are inserted before normal columns for the same index, so have to be handled first
			int nextExportColumnInsertIndex = exportColumns.at(exportOnlyColumnIndex).first;
			if (nextExportColumnInsertIndex == normalColumnIndex) {
				list.append(exportColumns.at(exportOnlyColumnIndex).second);
				exportOnlyColumnIndex++;
				continue;
			}
		}
		
		if (normalColumnIndex < numNormalColumns) {
			// At least one normal column left
			list.append(columns.at(normalColumnIndex));
			normalColumnIndex++;
			continue;
		}
		
		assert(false);
	}
	
	return list;
}

/**
 * Returns the composite column at the given index.
 * 
 * @param columnIndex	The index of the column to return.
 * @return				The composite column at the given index.
 */
const CompositeColumn* CompositeTable::getColumnAt(int columnIndex) const
{
	return columns.at(columnIndex);
}

/**
 * Returns the composite column used only for exports at the given index of the export column list.
 * 
 * @param columnIndex	The index in the export-only column list of the column to return.
 * @return				The composite column at the given index in the export-only column list.
 */
const CompositeColumn* CompositeTable::getExportOnlyColumnAt(int columnIndex) const
{
	return exportColumns.at(columnIndex).second;
}

/**
 * Returns the composite column with the given internal name.
 * 
 * @pre The column is a normal or filter-only column, not an export-only column.
 * 
 * @param columnName	The internal name of the column to return.
 * @return				The composite column with the given name, or a nullptr.
 */
const CompositeColumn* CompositeTable::getColumnByName(const QString& columnName) const
{
	for (const CompositeColumn* const column : columns) {
		if (column->name == columnName) return column;
	}
	return nullptr;
}

/**
 * Returns the index of the given normal or filter-only column.
 * 
 * @pre The given column is a normal or filter-only column, not an export-only column.
 * 
 * @param column	The normal or filter-only column to return the index of.
 * @return			The index of the given normal or filter-only column.
 */
int CompositeTable::getIndexOf(const CompositeColumn* column) const
{
	return columns.indexOf(column);
}

/**
 * Returns the index of the given export-only composite column.
 * 
 * @pre The given column is an export-only column, not a normal or filter-only column.
 * 
 * @param column	The composite column to return the export-only column index of.
 * @return			The export-only column index of the given composite column.
 */
int CompositeTable::getExportIndexOf(const CompositeColumn* column) const
{
	for (int exportColumnIndex = 0; exportColumnIndex < exportColumns.size(); exportColumnIndex++) {
		if (exportColumns.at(exportColumnIndex).second == column) return exportColumnIndex;
	}
	return -1;
}

/**
 * Returns a set with the names of all normal (not export-only or filter-only) columns.
 * 
 * @return	The names of all normal columns.
 */
QSet<QString> CompositeTable::getNormalColumnNameSet() const
{
	QSet<QString> columnNameSet;
	for (int columnIndex = 0; columnIndex < getNumberOfNormalColumns(); columnIndex++) {
		columnNameSet.insert(columns.at(columnIndex)->name);
	}
	return columnNameSet;
}

/**
 * Returns the database table this composite table is based on.
 * 
 * @return	The database table this composite table is based on.
 */
const NormalTable* CompositeTable::getBaseTable() const
{
	return baseTable;
}



/**
 * Returns the number of cells the table will contain once it is initialized.
 * 
 * @return	The total number of cells which need to be initialized.
 */
int CompositeTable::getNumberOfCellsToInit() const
{
	assert(!bufferInitialized);
	return baseTable->getNumberOfRows() * columns.size();
}

/**
 * Initializes the buffer and the order buffer.
 * 
 * Used to initialize the table when loading a project.
 * 
 * @param progressDialog			A progress dialog to update while initializing the buffer.
 * @param deferCompute				Whether to defer computing the contents of the cells until further notice.
 * @param autoResizeAfterCompute	The table view to automatically resize after the buffer is computed.
 */
void CompositeTable::initBuffer(QProgressDialog* progressDialog, bool deferCompute, QTableView* autoResizeAfterCompute)
{
	assert(!bufferInitialized);
	assert(buffer.isEmpty() && viewOrder.isEmpty());
	assert(dirtyColumns.isEmpty());
	
	for (const CompositeColumn* column : columns) {
		dirtyColumns.insert(column);
	}
	QSet<const CompositeColumn*> columnsToUpdate = getColumnsToUpdate();
	
	// Initialize cells and compute their contents for most columns
	int numberOfRows = baseTable->getNumberOfRows();
	for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(numberOfRows); bufferRowIndex++) {
		QList<QVariant>* newRow = new QList<QVariant>();
		for (const CompositeColumn* const column : columns) {
			bool noUpdateColumn = !columnsToUpdate.contains(column);
			bool computeWholeColumn = column->cellsAreInterdependent;
			
			QVariant newCell = QVariant();
			if (!deferCompute && !noUpdateColumn && !computeWholeColumn) {
				newCell = computeCellContent(bufferRowIndex, column->getIndex());
			}
			newRow->append(newCell);
			
			if (progressDialog && !computeWholeColumn) progressDialog->setValue(progressDialog->value() + 1);
		}
		buffer.appendRow(newRow);
	}
	
	// For columns which have to be computed as a whole, do that now
	for (const CompositeColumn* const column : columns) {
		bool noUpdateColumn = !columnsToUpdate.contains(column);
		bool computeWholeColumn = column->cellsAreInterdependent;
		if (noUpdateColumn || !computeWholeColumn) continue;
		
		QList<QVariant> cells = computeWholeColumnContent(column->getIndex());
		for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(baseTable->getNumberOfRows()); bufferRowIndex++) {
			buffer.replaceCell(bufferRowIndex, column->getIndex(), cells.at(bufferRowIndex.get()));
			if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
		}
	}
	
	bufferInitialized = true;
	rebuildOrderBuffer();
	
	if (!deferCompute) {
		dirtyColumns.subtract(columnsToUpdate);
	}
	
	if (deferCompute) {
		tableToAutoResizeAfterCompute = autoResizeAfterCompute;
	} else if (autoResizeAfterCompute) {
		autoResizeAfterCompute->resizeColumnsToContents();
	}
}

/**
 * Rebuilds the order buffer after changes which could affect the sorting or filtering.
 * 
 * Repopulates the order buffer from scratch, which means collecting all rows from the base table
 * without restriction. Then, all filters are applied to the order buffer, and finally the order
 * buffer is sorted according to the current sorting.
 * 
 * The repopulation step can be skipped if it is known that all rows which should be shown from
 * here on are already in the order buffer, i.e., no previously applied filters have been removed
 * or relaxed in any way. In practice, this should not be done if the filters have changed at all.
 * 
 * @param skipRepopulate	Whether to skip repopulating the order buffer.
 */
void CompositeTable::rebuildOrderBuffer(bool skipRepopulate)
{
	assert(bufferInitialized);
	
	beginResetModel();
	
	// Fill order buffer
	if (!skipRepopulate) {
		viewOrder.clear();
		for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(buffer.numRows()); bufferRowIndex++) {
			viewOrder.append(bufferRowIndex);
		}
	}
	
	// Filter order buffer
	for (const Filter& filter : qAsConst(currentFilters)) {
		filter.column->applySingleFilter(filter, viewOrder);
	}
	
	// Sort order buffer
	performSort(getCurrentSorting(), false);
	
	endResetModel();
}

/**
 * Returns the number of cells which need to be updated.
 * 
 * This includes every column which is dirty and either not hidden or used for sorting or filtering.
 * 
 * @return	The set of columns which need to be updated.
 */
QSet<const CompositeColumn*> CompositeTable::getColumnsToUpdate() const
{
	QSet<const CompositeColumn*> columnsToUpdate = QSet<const CompositeColumn*>(dirtyColumns);
	columnsToUpdate.subtract(hiddenColumns);
	if (currentSorting.column) columnsToUpdate.insert(currentSorting.column);
	for (const Filter& filter : currentFilters) {
		assert(filter.column);
		columnsToUpdate.insert(filter.column);
	}
	return columnsToUpdate;
}

/**
 * Returns the number of cells which need to be updated.
 * 
 * This does not include columns which are marked dirty but also hidden.
 * 
 * @return	The number of cells which need to be updated.
 */
int CompositeTable::getNumberOfCellsToUpdate() const
{
	assert(bufferInitialized);
	return getColumnsToUpdate().size() * buffer.numRows();
}

/**
 * Updates the contents of the given columns in the buffer, if they are marked dirty.
 * 
 * After updating, the updated columns are removed from the set of dirty columns, but the order
 * buffer is not rebuilt, therefore performSort() is not called.
 * 
 * @param columnsToUpdate			The columns to potentially update.
 * @param runAfterEachCellUpdate	A lambda function to be run every time a cell value has been updated.
 */
void CompositeTable::updateBufferColumns(QSet<const CompositeColumn*> columnsToUpdate, std::function<void()> runAfterEachCellUpdate)
{
	assert(bufferInitialized);
	
	columnsToUpdate.intersect(dirtyColumns);
	if (columnsToUpdate.isEmpty()) return;
	
	if (!runAfterEachCellUpdate) runAfterEachCellUpdate = []() {};
	
	for (const CompositeColumn* column : qAsConst(columnsToUpdate)) {
		int columnIndex = column->getIndex();
		bool computeWholeColumn = column->cellsAreInterdependent;
		if (!computeWholeColumn) {
			for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(buffer.numRows()); bufferRowIndex++) {
				QVariant newContent = computeCellContent(bufferRowIndex, columnIndex);
				buffer.replaceCell(bufferRowIndex, columnIndex, newContent);
				
				runAfterEachCellUpdate();
			}
		}
		else {
			QList<QVariant> cells = computeWholeColumnContent(columnIndex);
			for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(baseTable->getNumberOfRows()); bufferRowIndex++) {
				buffer.replaceCell(bufferRowIndex, columnIndex, cells.at(bufferRowIndex.get()));
				
				runAfterEachCellUpdate();
			}
		}
		
		QModelIndex topLeftIndex		= index(0, columnIndex);
		QModelIndex bottomRightIndex	= index(viewOrder.numRows(), columnIndex);
		Q_EMIT dataChanged(topLeftIndex, bottomRightIndex);
	}
	
	dirtyColumns.subtract(columnsToUpdate);
}

/**
 * Updates the contents of all columns which are marked dirty.
 * 
 * After updating the buffer, the order buffer is rebuilt and the model is notified of the changes.
 * 
 * @param runAfterEachCellUpdate	A lambda function to be run every time a cell value has been updated.
 */
void CompositeTable::updateBothBuffers(std::function<void()> runAfterEachCellUpdate)
{
	assert(bufferInitialized);
	
	QSet<const CompositeColumn*> columnsToUpdate = getColumnsToUpdate();
	if (columnsToUpdate.isEmpty()) return;
	
	// Update the scheduled columns
	updateBufferColumns(columnsToUpdate, runAfterEachCellUpdate);
	
	// Rebuild order buffer if necessary
	bool orderBufferDirty = columnsToUpdate.contains(currentSorting.column);
	for (const Filter& filter : qAsConst(currentFilters)) {
		orderBufferDirty |= columnsToUpdate.contains(filter.column);
	}
	if (orderBufferDirty) {
		rebuildOrderBuffer(false);
	}
	
	if (tableToAutoResizeAfterCompute) {
		tableToAutoResizeAfterCompute->resizeColumnsToContents();
		tableToAutoResizeAfterCompute = nullptr;
	}
}

/**
 * Completely resets the buffer and the order buffer.
 * 
 * Used when closing a project.
 */
void CompositeTable::resetBuffer()
{
	beginResetModel();
	viewOrder.clear();
	endResetModel();
	
	buffer.reset();
	bufferInitialized = false;
	dirtyColumns.clear();
	hiddenColumns.clear();
}

/**
 * Returns the index at which the item shown at the given view row is stored in the buffer.
 * 
 * @param viewRowIndex	The index of the view row to return the buffer index for.
 * @return				The buffer index of the item shown at the given view row.
 */
BufferRowIndex CompositeTable::getBufferRowIndexForViewRow(ViewRowIndex viewRowIndex) const
{
	return viewOrder.getBufferRowIndexForViewRow(viewRowIndex);
}

/**
 * Returns the index of the view row which shows the item at the given buffer index, if any.
 * 
 * If the item at the given buffer index is not shown in the view (filtered out), an invalid
 * ViewRowIndex is returned.
 * 
 * @param bufferRowIndex	The index in the buffer of the item to return the view index for.
 * @return					The view index of the item, or an invalid ViewRowIndex if it is not shown.
 */
ViewRowIndex CompositeTable::findViewRowIndexForBufferRow(BufferRowIndex bufferRowIndex) const
{
	assert(bufferInitialized);
	if (bufferRowIndex.isInvalid(buffer.numRows())) return ViewRowIndex();
	return viewOrder.findViewRowIndexForBufferRow(bufferRowIndex);
}



/**
 * Returns the raw (unformatted, not for UI display) value of the cell at the given buffer row and
 * column indices.
 * 
 * @param bufferRowIndex	The index of the buffer row to return the value for.
 * @param column			The column to return the value for.
 * @return					The raw value of the cell at the given buffer row and column index.
 */
QVariant CompositeTable::getRawValue(BufferRowIndex bufferRowIndex, const CompositeColumn* column)
{
	assert(bufferInitialized);
	assert(columns.contains(column));
	assert(bufferRowIndex.isValid(buffer.numRows()));
	
	QVariant result;
	if (dirtyColumns.contains(column)) {
		if (column->cellsAreInterdependent) {
			// Have to compute whole column anyway, might as well update the buffer
			updateBufferColumns({column});
			result = buffer.getCell(bufferRowIndex, column->getIndex());
		} else {
			// Compute single cell instead of updating buffer for entire column to save time
			result = column->computeValueAt(bufferRowIndex);
		}
	} else {
		// Buffer is up to date
		result = buffer.getCell(bufferRowIndex, column->getIndex());
	}
	
	return result.isNull() ? QVariant() : result;
}

/**
 * Returns the formatted (for UI display) value of the cell at the given buffer row and column
 * indices.
 * 
 * @param bufferRowIndex	The index of the buffer row to return the value for.
 * @param column			The column to return the value for.
 * @return					The formatted value of the cell at the given buffer row and column index.
 */
QVariant CompositeTable::getFormattedValue(BufferRowIndex bufferRowIndex, const CompositeColumn* column)
{
	return column->toFormattedTableContent(getRawValue(bufferRowIndex, column));
}



/**
 * Returns the current sorting.
 * 
 * @return	The current sorting as a pair of the column to sort by and the sort order.
 */
SortingPass CompositeTable::getCurrentSorting() const
{
	return currentSorting;
}



/**
 * Sets filters without applying them, only to be used during initialization.
 * 
 * @param filters	The filters to apply once the table is populated.
 */
void CompositeTable::setInitialFilters(QSet<Filter> filters)
{
	assert(!bufferInitialized && viewOrder.isEmpty());
	currentFilters = filters;
}

/**
 * Applies the given filters to the table, updating the order buffer and attempting to restore the
 * selection in the view.
 * 
 * @param filters	The filters to apply.
 */
void CompositeTable::applyFilters(QSet<Filter> filters)
{
	ViewRowIndex previouslySelectedViewRowIndex = ViewRowIndex(tableView->currentIndex().row());
	BufferRowIndex previouslySelectedBufferRowIndex = getBufferRowIndexForViewRow(previouslySelectedViewRowIndex);
	
	bool skipRepopulate = currentFilters.isEmpty();
	currentFilters = filters;
	updateBufferColumns(getColumnsToUpdate());	// Filter column(s) might need to be updated if hidden
	rebuildOrderBuffer(skipRepopulate);
	
	// Restore selection
	if (previouslySelectedBufferRowIndex.isValid()) {
		ViewRowIndex newViewRowIndex = findViewRowIndexForBufferRow(previouslySelectedBufferRowIndex);
		QModelIndex modelIndex = index(newViewRowIndex.get(), 0);
		tableView->setCurrentIndex(modelIndex);
		tableView->scrollTo(modelIndex);
	}
}

/**
 * Removes all applied filters from the table and performs the necessary updates.
 */
void CompositeTable::clearFilters()
{
	applyFilters({});
}

/**
 * Returns the currently applied filters.
 * 
 * @return	The set of currently applied filters.
 */
QSet<Filter> CompositeTable::getCurrentFilters() const
{
	return currentFilters;
}

/**
 * Indicates whether any filters are currently applied to the table.
 * 
 * @return	True if any filters are currently applied, false otherwise.
 */
bool CompositeTable::filterIsActive() const
{
	return !currentFilters.isEmpty();
}



/**
 * Marks the given column as hidden.
 * 
 * A hidden column is not updated unless it is used for sorting and/or filtering.
 * 
 * @param columnIndex	The index of the column to mark as hidden.
 */
void CompositeTable::markColumnHidden(int columnIndex)
{
	hiddenColumns.insert(getColumnAt(columnIndex));
}

/**
 * Marks the given column as not hidden, but does not perform an automatic buffer update.
 * 
 * @param columnIndex	The index of the column to mark as not hidden.
 */
void CompositeTable::markColumnUnhidden(int columnIndex)
{
	hiddenColumns.remove(getColumnAt(columnIndex));
}

/**
 * Marks all columns as not hidden, but does not perform an automatic buffer update.
 */
void CompositeTable::markAllColumnsUnhidden()
{
	hiddenColumns.clear();
}



/**
 * Sets whether the table should update its columns immediately when notified of changes in the
 * database, or defer the updates.
 * 
 * If the new setting is to update immediately, the buffer is then updated where necessary. A
 * progress dialog can be passed to track progress during the update process.
 * 
 * @param updateImmediately	Whether to update columns immediately after changes.
 * @param progress			A progress dialog to update while updating the buffer.
 */
void CompositeTable::setUpdateImmediately(bool updateImmediately, QProgressDialog* progress)
{
	this->updateImmediately = updateImmediately;
	
	auto progressUpdateLambda = [progress] () {
		progress->setValue(progress->value() + 1);
	};
	if (updateImmediately && bufferInitialized) {
		updateBothBuffers(progressUpdateLambda);
	}
}

/**
 * Notifies the composite table that a row has been inserted into its base table.
 * 
 * @param bufferRowIndex	The index in the buffer of the new row.
 */
void CompositeTable::bufferRowJustInserted(BufferRowIndex bufferRowIndex)
{
	assert(bufferInitialized);
	
	QList<QVariant>* newRow = new QList<QVariant>();
	for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
		QVariant newCell;
		if (getColumnAt(columnIndex)->cellsAreInterdependent) {
			// Computing single cell is expensive and whole column will be completely recomputed anyway
			// => leave cell empty for now
			newCell = QVariant();
		} else {
			newCell = computeCellContent(bufferRowIndex, columnIndex);
		}
		newRow->append(newCell);
	}
	buffer.insertRow(bufferRowIndex, newRow);
	
	rebuildOrderBuffer(false);
}

/**
 * Notifies the composite table that a row is about to be removed from its base table.
 * 
 * @param bufferRowIndex	The index in the buffer of the row which will be removed.
 */
void CompositeTable::bufferRowAboutToBeRemoved(BufferRowIndex bufferRowIndex)
{
	assert(bufferInitialized);
	
	ViewRowIndex viewRowIndex = findViewRowIndexForBufferRow(bufferRowIndex);
	if (viewRowIndex.isValid()) {
		beginRemoveRows(QModelIndex(), viewRowIndex.get(), viewRowIndex.get());
		viewOrder.removeViewRow(viewRowIndex);
		endRemoveRows();
		
		// Update order buffer
		for (ViewRowIndex viewRow = ViewRowIndex(0); viewRow.isValid(viewOrder.numRows()); viewRow++) {
			BufferRowIndex currentBufferRowIndex = viewOrder.getBufferRowIndexForViewRow(viewRow);
			if (currentBufferRowIndex > bufferRowIndex) {
				viewOrder.replaceBufferRowIndexAtViewRowIndex(viewRow, currentBufferRowIndex - 1);
			}
		}
	}
	
	buffer.removeRow(bufferRowIndex);
}

/**
 * Notifies the composite table that data in a column it depends on has changed.
 * 
 * @param columnIndex	The index of the affected column among the composite table's columns.
 */
void CompositeTable::announceChangesUnderColumn(int columnIndex)
{
	dirtyColumns.insert(columns.at(columnIndex));
	if (updateImmediately) updateBothBuffers();
}



/**
 * For the QAbstractItemModel implementation, returns the number of rows in the table under the
 * given parent.
 * 
 * @param parent	The parent model index, assumed to be invalid.
 * @return			The number of rows in the table.
 */
int CompositeTable::rowCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return viewOrder.numRows();
}

/**
 * For the QAbstractItemModel implementation, returns the number of visible columns in the table
 * under the given parent.
 * 
 * @param parent	The parent model index, assumed to be invalid.
 * @return			The number of columns in the table.
 */
int CompositeTable::columnCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return getNumberOfNormalColumns();
}

/**
 * For the QAbstractItemModel implementation, returns the data for the given role and section in
 * the header with the specified orientation.
 * 
 * For horizontal headers, the section number corresponds to the column number. Similarly, for
 * vertical headers, the section number corresponds to the row number.
 * 
 * @param section		The index of the section to return the data for.
 * @param orientation	The orientation of the header (horizontal or vertical).
 * @param role			The Qt::ItemDataRole for which to return the data.
 * @return				The data for the given role and section in the header with the specified orientation.
 */
QVariant CompositeTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Orientation::Vertical) {
		assert(section >= 0 && section < viewOrder.numRows());
		switch (role) {
		case Qt::TextAlignmentRole:	return Qt::AlignRight;
		case Qt::DisplayRole:		return section + 1;
		}
		return QVariant();
	}
	
	if (role != Qt::DisplayRole) return QVariant();
	
	assert(section >= 0 && section < columns.size());
	return columns.at(section)->uiName;
}

/**
 * For the QAbstractItemModel implementation, returns the data for the given role and index in the
 * table.
 * 
 * This function defines the shape in which data is actually displayed in the table. For example,
 * boolean values can be displayed as checkboxes using the Qt::CheckStateRole. Alignment is also
 * defined here.
 * 
 * @param index	The model index to return the data for. Assumed to have no valid parent.
 * @param role	The Qt::ItemDataRole for which to return the data.
 * @return		The data for the given role and index in the table.
 */
QVariant CompositeTable::data(const QModelIndex& index, int role) const
{
	assert(bufferInitialized);
	assert(!index.parent().isValid());
	
	ViewRowIndex viewRowIndex = ViewRowIndex(index.row());
	assert(viewRowIndex.isValid(viewOrder.numRows()));
	BufferRowIndex bufferRowIndex = viewOrder.getBufferRowIndexForViewRow(viewRowIndex);
	assert(bufferRowIndex.isValid(buffer.numRows()));
	
	int columnIndex = index.column();
	assert(columnIndex >= 0 && columnIndex < columns.size());
	const CompositeColumn* column = columns.at(columnIndex);
	
	if (role == Qt::TextAlignmentRole) {
		return column->alignment;
	}
	
	int relevantRole = column->contentType == Bit ? Qt::CheckStateRole : Qt::DisplayRole;
	if (role != relevantRole) return QVariant();
	
	QVariant result = buffer.getCell(bufferRowIndex, columnIndex);
	
	if (result.isNull()) return QVariant();
	
	if (column->contentType == Bit) {
		assert(role == Qt::CheckStateRole);
		return result.toBool() ? Qt::Checked : Qt::Unchecked;
	}
	
	return column->toFormattedTableContent(result);
}

/**
 * For the QAbstractItemModel implementation, sorts the table by the given column and order.
 * 
 * This function is called by the view when the user clicks on a column header to sort by that
 * column. It looks up the column which was clicked and delegates the sorting to
 * performSortByColumn().
 * 
 * @param columnIndex	The index of the visible column to sort by.
 * @param order			The order to sort by (ascending or descending).
 */
void CompositeTable::sort(int columnIndex, Qt::SortOrder order)
{
	assert(columnIndex >= 0 && columnIndex < columns.size());
	const CompositeColumn* const column = columns.at(columnIndex);
	
	if (bufferInitialized) updateBufferColumns({column});	// Sort column might need to be updated if hidden
	
	const SortingPass previousSort = currentSorting;
	currentSorting = {column, order};
	
	performSort(previousSort, true);
}

/**
 * Sorts the visible rows in the table by the given column and order.
 * 
 * Once the table is initialized, the table does not need to be resorted from scratch if the column
 * to sort by does not change (i.e., it is the same as in the current sorting). Then, either
 * nothing needs to be done if the order is *also* the same as in the current sorting, or the order
 * can simply be reversed if the order is opposite to the current sorting.
 * 
 * @param previousSort			The column and order the table was sorted by before this call.
 * @param allowPassAndReverse	Whether to allow shortcuts in resorting. Do not use on initial sort.
 */
void CompositeTable::performSort(SortingPass previousSort, bool allowPassAndReverse)
{
	assert(currentSorting.column);
	assert(tableView);
	
	ViewRowIndex previouslySelectedViewRowIndex = ViewRowIndex(tableView->currentIndex().row());
	BufferRowIndex previouslySelectedBufferRowIndex = getBufferRowIndexForViewRow(previouslySelectedViewRowIndex);
	
	if (allowPassAndReverse && currentSorting.column == previousSort.column) {
		if (currentSorting.order == previousSort.order) return;
		
		viewOrder.reverse();
	}
	else {
		auto comparator = [this](const BufferRowIndex& index1, const BufferRowIndex& index2) {
			QVariant value1 = currentSorting.column->getRawValueAt(index1);
			QVariant value2 = currentSorting.column->getRawValueAt(index2);
			
			if (currentSorting.order == Qt::AscendingOrder) {
				return currentSorting.column->compare(value1, value2);
			} else {
				return currentSorting.column->compare(value2, value1);
			}
		};
		
		viewOrder.sortBy(comparator);
	}
	
	// Restore selection
	if (previouslySelectedBufferRowIndex.isValid()) {
		ViewRowIndex newViewRowIndex = findViewRowIndexForBufferRow(previouslySelectedBufferRowIndex);
		QModelIndex modelIndex = index(newViewRowIndex.get(), 0);
		tableView->setCurrentIndex(modelIndex);
		tableView->scrollTo(modelIndex);
	}
	
	// Notify model users (views)
	QModelIndex topLeftIndex		= index(0, 0);
	QModelIndex bottomRightIndex	= index(viewOrder.numRows() - 1, columns.size() - 1);
	Q_EMIT dataChanged(topLeftIndex, bottomRightIndex);
	//headerDataChanged(Qt::Vertical, 0, bufferOrder.size() - 1);
}



/**
 * Computes the value of the cell at the given buffer row and column indices.
 * 
 * @param bufferRowIndex	The index of the buffer row to compute the value for.
 * @param columnIndex		The index of the column to compute the value for.
 * @return					The computed raw value of the cell at the given buffer row and column indices.
 */
QVariant CompositeTable::computeCellContent(BufferRowIndex bufferRowIndex, int columnIndex) const
{
	assert(bufferRowIndex.isValid(baseTable->getNumberOfRows()));
	assert(columnIndex >= 0 && columnIndex < columns.size());
	
	const CompositeColumn* column = columns.at(columnIndex);
	QVariant result = column->computeValueAt(bufferRowIndex);
	
	if (!result.isValid()) return QVariant();
	
	return result;
}

/**
 * Computes the value of all cells in the column together.
 * 
 * Used for composite columns with interdependent cells, such as IndexCompositeColumn.
 * 
 * @param columnIndex	The index of the column to compute the values for.
 * @return				The list of all computed raw values for the cells in the column.
 */
QList<QVariant> CompositeTable::computeWholeColumnContent(int columnIndex) const
{
	const CompositeColumn* column = columns.at(columnIndex);
	QList<QVariant> cells = column->computeWholeColumn();
	assert(cells.size() == buffer.numRows());
	return cells;
}



/**
 * Returns a pointer to the project settings object.
 * 
 * @return	The project settings object.
 */
ProjectSettings* CompositeTable::getProjectSettings() const
{
	return db->projectSettings;
}
