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

#include "composite_table.h"



CompositeTable::CompositeTable(Database* db, NormalTable* baseTable, QTableView* tableView) :
		QAbstractTableModel(),
		db(db),
		baseTable(baseTable),
		tableView(tableView),
		columns(QList<const CompositeColumn*>()),
		firstHiddenColumnIndex(-1),
		buffer(TableBuffer()),
		viewOrder(ViewOrderBuffer()),
		currentSorting({nullptr, Qt::AscendingOrder}),
		currentFilters(QSet<Filter>()),
		columnsToUpdate(QSet<const CompositeColumn*>()),
		updateImmediately(false),
		tableToAutoResizeAfterCompute(nullptr),
		name(baseTable->name)
{
	baseTable->setRowChangeListener(this);
}

CompositeTable::~CompositeTable()
{
	qDeleteAll(columns);
	qDeleteAll(buffer);
}



void CompositeTable::addColumn(const CompositeColumn* column, bool hidden)
{
	if (hidden) {
		if (firstHiddenColumnIndex < 0) firstHiddenColumnIndex = columns.size();
	} else {
		assert(firstHiddenColumnIndex < 0);
	}
	
	columns.append(column);
	
	// Register as change listener at all underlying columns
	const QSet<Column* const> underlyingColumns = column->getAllUnderlyingColumns();
	for (Column* underlyingColumn : underlyingColumns) {
		underlyingColumn->registerChangeListener(column);
	}
}

const CompositeColumn* CompositeTable::getColumnAt(int columnIndex) const
{
	return columns.at(columnIndex);
}

int CompositeTable::getIndexOf(const CompositeColumn* column) const
{
	return columns.indexOf(column);
}

const NormalTable* CompositeTable::getBaseTable() const
{
	return baseTable;
}



int CompositeTable::getNumberOfCellsToInit() const
{
	return baseTable->getNumberOfRows() * columns.size();
}

void CompositeTable::initBuffer(QProgressDialog* progressDialog, bool deferCompute, QTableView* autoResizeAfterCompute)
{
	assert(buffer.isEmpty() && viewOrder.isEmpty());
	
	int numberOfRows = baseTable->getNumberOfRows();
	
	// Initialize cells and compute their contents for most columns
	for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(numberOfRows); bufferRowIndex++) {
		QList<QVariant>* newRow = new QList<QVariant>();
		for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
			bool computeWholeColumn = getColumnAt(columnIndex)->cellsAreInterdependent;
			QVariant newCell = QVariant();
			if (!deferCompute && !computeWholeColumn) {
				newCell = computeCellContent(bufferRowIndex, columnIndex);
			}
			newRow->append(newCell);
			
			if (progressDialog && !computeWholeColumn) progressDialog->setValue(progressDialog->value() + 1);
		}
		buffer.appendRow(newRow);
	}
	
	// For columns which have to be computed as a whole, do that now
	for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
		bool computeWholeColumn = getColumnAt(columnIndex)->cellsAreInterdependent;
		if (!computeWholeColumn) continue;
		
		QList<QVariant> cells = computeWholeColumnContent(columnIndex);
		for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(baseTable->getNumberOfRows()); bufferRowIndex++) {
			buffer.replaceCell(bufferRowIndex, columnIndex, cells.at(bufferRowIndex.get()));
			if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
		}
	}
	
	rebuildOrderBuffer();
	
	columnsToUpdate.clear();
	if (deferCompute) {
		for (const CompositeColumn* column : columns) columnsToUpdate.insert(column);
		tableToAutoResizeAfterCompute = autoResizeAfterCompute;
	} else {
		if (autoResizeAfterCompute) autoResizeAfterCompute->resizeColumnsToContents();
	}
}

void CompositeTable::rebuildOrderBuffer(bool skipRepopulate)
{
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
	performSortByColumn(getCurrentSorting().first, getCurrentSorting().second, false);
	
	endResetModel();
}

int CompositeTable::getNumberOfCellsToUpdate() const
{
	return columnsToUpdate.size() * buffer.numRows();
}

void CompositeTable::updateBuffer(QProgressDialog* progressDialog)
{
	if (columnsToUpdate.isEmpty()) return;
	
	for (const CompositeColumn* column : qAsConst(columnsToUpdate)) {
		int columnIndex = column->getIndex();
		bool computeWholeColumn = column->cellsAreInterdependent;
		if (!computeWholeColumn) {
			for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(buffer.numRows()); bufferRowIndex++) {
				QVariant newContent = computeCellContent(bufferRowIndex, columnIndex);
				buffer.replaceCell(bufferRowIndex, columnIndex, newContent);
				
				if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
			}
		}
		else {
			QList<QVariant> cells = computeWholeColumnContent(columnIndex);
			for (BufferRowIndex bufferRowIndex = BufferRowIndex(0); bufferRowIndex.isValid(baseTable->getNumberOfRows()); bufferRowIndex++) {
				buffer.replaceCell(bufferRowIndex, columnIndex, cells.at(bufferRowIndex.get()));
				if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
			}
		}
		
		QModelIndex topLeftIndex		= index(0, columnIndex);
		QModelIndex bottomRightIndex	= index(viewOrder.numRows(), columnIndex);
		Q_EMIT dataChanged(topLeftIndex, bottomRightIndex);
	}
	
	rebuildOrderBuffer(false);
	
	columnsToUpdate.clear();
	
	if (tableToAutoResizeAfterCompute) {
		tableToAutoResizeAfterCompute->resizeColumnsToContents();
		tableToAutoResizeAfterCompute = nullptr;
	}
}

void CompositeTable::resetBuffer()
{
	beginResetModel();
	viewOrder.clear();
	endResetModel();
	
	buffer.reset();
}

BufferRowIndex CompositeTable::getBufferRowIndexForViewRow(ViewRowIndex viewRowIndex) const
{
	return viewOrder.getBufferRowIndexForViewRow(viewRowIndex);
}

ViewRowIndex CompositeTable::findViewRowIndexForBufferRow(BufferRowIndex bufferRowIndex) const
{
	if (bufferRowIndex.isInvalid(buffer.numRows())) return ViewRowIndex();
	return viewOrder.findViewRowIndexForBufferRow(bufferRowIndex);
}



QVariant CompositeTable::getRawValue(BufferRowIndex bufferRowIndex, const CompositeColumn* column) const
{
	assert(columns.contains(column));
	assert(bufferRowIndex.isValid(buffer.numRows()));
	QVariant result = buffer.getCell(bufferRowIndex, column->getIndex());
	return result.isNull() ? QVariant() : result;
}

QVariant CompositeTable::getFormattedValue(BufferRowIndex bufferRowIndex, const CompositeColumn* column) const
{
	return column->toFormattedTableContent(getRawValue(bufferRowIndex, column));
}



QPair<const CompositeColumn*, Qt::SortOrder> CompositeTable::getCurrentSorting() const
{
	return currentSorting;
}



void CompositeTable::setInitialFilters(QSet<Filter> filters)
{
	assert(buffer.isEmpty() && viewOrder.isEmpty());
	currentFilters = filters;
}

void CompositeTable::applyFilters(QSet<Filter> filters)
{
	ViewRowIndex previouslySelectedViewRowIndex = ViewRowIndex(tableView->currentIndex().row());
	BufferRowIndex previouslySelectedBufferRowIndex = getBufferRowIndexForViewRow(previouslySelectedViewRowIndex);
	
	bool skipRepopulate = currentFilters.isEmpty();
	currentFilters = filters;
	rebuildOrderBuffer(skipRepopulate);
	
	// Restore selection
	if (previouslySelectedBufferRowIndex.isValid()) {
		ViewRowIndex newViewRowIndex = findViewRowIndexForBufferRow(previouslySelectedBufferRowIndex);
		QModelIndex modelIndex = index(newViewRowIndex.get(), 0);
		tableView->setCurrentIndex(modelIndex);
		tableView->scrollTo(modelIndex);
	}
}

void CompositeTable::clearFilters()
{
	applyFilters({});
}

QSet<Filter> CompositeTable::getCurrentFilters() const
{
	return currentFilters;
}

bool CompositeTable::filterIsActive() const
{
	return !currentFilters.isEmpty();
}



void CompositeTable::setUpdateImmediately(bool updateImmediately, QProgressDialog* progress)
{
	this->updateImmediately = updateImmediately;
	if (updateImmediately) updateBuffer(progress);
}

void CompositeTable::bufferRowJustInserted(BufferRowIndex bufferRowIndex)
{
	QList<QVariant>* newRow = new QList<QVariant>();
	for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
		newRow->append(computeCellContent(bufferRowIndex, columnIndex));
	}
	buffer.insertRow(bufferRowIndex, newRow);
	
	rebuildOrderBuffer(false);
}

void CompositeTable::bufferRowAboutToBeRemoved(BufferRowIndex bufferRowIndex)
{
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

void CompositeTable::announceChangesUnderColumn(int columnIndex)
{
	columnsToUpdate.insert(columns.at(columnIndex));
	if (updateImmediately) updateBuffer(nullptr);
}



int CompositeTable::rowCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return viewOrder.numRows();
}

int CompositeTable::columnCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	if (firstHiddenColumnIndex >= 0) return firstHiddenColumnIndex;
	return columns.size();
}

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

QVariant CompositeTable::data(const QModelIndex& index, int role) const
{
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

void CompositeTable::sort(int columnIndex, Qt::SortOrder order)
{
	assert(columnIndex >= 0 && columnIndex < columns.size());
	const CompositeColumn* const column = columns.at(columnIndex);
	
	performSortByColumn(column, order, true);
}

void CompositeTable::performSortByColumn(const CompositeColumn* column, Qt::SortOrder order, bool allowPassAndReverse)
{
	assert(column);
	assert(tableView);
	
	ViewRowIndex previouslySelectedViewRowIndex = ViewRowIndex(tableView->currentIndex().row());
	BufferRowIndex previouslySelectedBufferRowIndex = getBufferRowIndexForViewRow(previouslySelectedViewRowIndex);
	
	if (allowPassAndReverse && column == currentSorting.first) {
		if (order == currentSorting.second) return;
		
		viewOrder.reverse();
	}
	else {
		auto comparator = [&column, order](const BufferRowIndex& index1, const BufferRowIndex& index2) {
			QVariant value1 = column->getRawValueAt(index1);
			QVariant value2 = column->getRawValueAt(index2);
			
			if (order == Qt::AscendingOrder) {
				return column->compare(value1, value2);
			} else {
				return column->compare(value2, value1);
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
	
	currentSorting = {column, order};
}



QVariant CompositeTable::computeCellContent(BufferRowIndex bufferRowIndex, int columnIndex) const
{
	assert(bufferRowIndex.isValid(baseTable->getNumberOfRows()));
	assert(columnIndex >= 0 && columnIndex < columns.size());
	
	const CompositeColumn* column = columns.at(columnIndex);
	QVariant result = column->computeValueAt(bufferRowIndex);
	
	if (!result.isValid()) return QVariant();
	
	return result;
}

QList<QVariant> CompositeTable::computeWholeColumnContent(int columnIndex) const
{
	const CompositeColumn* column = columns.at(columnIndex);
	QList<QVariant> cells = column->computeWholeColumn();
	assert(cells.size() == buffer.numRows());
	return cells;
}



ProjectSettings* CompositeTable::getProjectSettings() const
{
	return db->projectSettings;
}
