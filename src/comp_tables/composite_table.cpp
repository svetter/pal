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
		buffer(QList<QList<QVariant>*>()),
		bufferOrder(QList<int>()),
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
	assert(buffer.isEmpty() && bufferOrder.isEmpty());
	
	int numberOfRows = baseTable->getNumberOfRows();
	
	// Initialize cells and compute their contents for most columns
	for (int bufferRowIndex = 0; bufferRowIndex < numberOfRows; bufferRowIndex++) {
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
		buffer.append(newRow);
	}
	
	// For columns which have to be computed as a whole, do that now
	for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
		bool computeWholeColumn = getColumnAt(columnIndex)->cellsAreInterdependent;
		if (!computeWholeColumn) continue;
		
		QList<QVariant> cells = computeWholeColumnContent(columnIndex);
		for (int bufferRowIndex = 0; bufferRowIndex < baseTable->getNumberOfRows(); bufferRowIndex++) {
			buffer.at(bufferRowIndex)->replace(columnIndex, cells.at(bufferRowIndex));
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
		bufferOrder.clear();
		for (int bufferRowIndex = 0; bufferRowIndex < buffer.size(); bufferRowIndex++) {
			bufferOrder.append(bufferRowIndex);
		}
	}
	
	// Filter order buffer
	for (const Filter& filter : qAsConst(currentFilters)) {
		filter.column->applySingleFilter(filter, bufferOrder);
	}
	
	// Sort order buffer
	performSortByColumn(getCurrentSorting().first, getCurrentSorting().second, false);
	
	endResetModel();
}

int CompositeTable::getNumberOfCellsToUpdate() const
{
	return columnsToUpdate.size() * buffer.size();
}

void CompositeTable::updateBuffer(QProgressDialog* progressDialog)
{
	if (columnsToUpdate.isEmpty()) return;
	
	for (const CompositeColumn* column : qAsConst(columnsToUpdate)) {
		int columnIndex = column->getIndex();
		bool computeWholeColumn = column->cellsAreInterdependent;
		if (!computeWholeColumn) {
			for (int bufferRowIndex = 0; bufferRowIndex < buffer.size(); bufferRowIndex++) {
				QVariant newContent = computeCellContent(bufferRowIndex, columnIndex);
				buffer.at(bufferRowIndex)->replace(columnIndex, newContent);
				
				if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
			}
		}
		else {
			QList<QVariant> cells = computeWholeColumnContent(columnIndex);
			for (int bufferRowIndex = 0; bufferRowIndex < baseTable->getNumberOfRows(); bufferRowIndex++) {
				buffer.at(bufferRowIndex)->replace(columnIndex, cells.at(bufferRowIndex));
				if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
			}
		}
		
		QModelIndex topLeftIndex		= index(0, columnIndex);
		QModelIndex bottomRightIndex	= index(bufferOrder.size(), columnIndex);
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
	bufferOrder.clear();
	endResetModel();
	
	qDeleteAll(buffer);
	buffer.clear();
}

int CompositeTable::getBufferRowIndexForViewRow(int viewRowIndex) const
{
	if (viewRowIndex < 0 || viewRowIndex >= bufferOrder.size()) return -1;
	return bufferOrder.at(viewRowIndex);
}

int CompositeTable::findViewRowIndexForBufferRow(int bufferRowIndex) const
{
	if (bufferRowIndex < 0 || bufferRowIndex >= buffer.size()) return -1;
	return bufferOrder.indexOf(bufferRowIndex);
}



QVariant CompositeTable::getRawValue(int bufferRowIndex, const CompositeColumn* column) const
{
	assert(columns.contains(column));
	assert(bufferRowIndex >= 0 && bufferRowIndex < buffer.size());
	QVariant result = buffer.at(bufferRowIndex)->at(column->getIndex());
	return result.isNull() ? QVariant() : result;
}

QVariant CompositeTable::getFormattedValue(int bufferRowIndex, const CompositeColumn* column) const
{
	return column->toFormattedTableContent(getRawValue(bufferRowIndex, column));
}



QPair<const CompositeColumn*, Qt::SortOrder> CompositeTable::getCurrentSorting() const
{
	return currentSorting;
}



void CompositeTable::setInitialFilters(QSet<Filter> filters)
{
	assert(buffer.isEmpty() && bufferOrder.isEmpty());
	currentFilters = filters;
}

void CompositeTable::applyFilters(QSet<Filter> filters)
{
	bool skipRepopulate = currentFilters.isEmpty();
	currentFilters = filters;
	rebuildOrderBuffer(skipRepopulate);
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

void CompositeTable::bufferRowJustInserted(int bufferRowIndex)
{
	QList<QVariant>* newRow = new QList<QVariant>();
	for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
		newRow->append(computeCellContent(bufferRowIndex, columnIndex));
	}
	buffer.insert(bufferRowIndex, newRow);
	
	rebuildOrderBuffer(false);
}

void CompositeTable::bufferRowAboutToBeRemoved(int bufferRowIndex)
{
	int viewRowIndex = findViewRowIndexForBufferRow(bufferRowIndex);
	if (viewRowIndex >= 0) {
		beginRemoveRows(QModelIndex(), viewRowIndex, viewRowIndex);
		bufferOrder.removeAt(viewRowIndex);
		endRemoveRows();
		
		// Update order buffer
		for (int i = 0; i < bufferOrder.size(); i++) {
			int currentBufferRowIndex = bufferOrder.at(i);
			if (currentBufferRowIndex > bufferRowIndex) {
				bufferOrder.replace(i, currentBufferRowIndex - 1);
			}
		}
	}
	
	QList<QVariant>* rowToRemove = buffer.at(bufferRowIndex);
	buffer.removeAt(bufferRowIndex);
	delete rowToRemove;
}

void CompositeTable::announceChangesUnderColumn(int columnIndex)
{
	columnsToUpdate.insert(columns.at(columnIndex));
	if (updateImmediately) updateBuffer(nullptr);
}



int CompositeTable::rowCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return bufferOrder.size();
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
		assert(section >= 0 && section < bufferOrder.size());
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
	
	int viewRowIndex = index.row();
	assert(viewRowIndex >= 0 && viewRowIndex < bufferOrder.size());
	int bufferRowIndex = bufferOrder.at(viewRowIndex);
	assert(bufferRowIndex >= 0 && bufferRowIndex < buffer.size());
	
	int columnIndex = index.column();
	assert(columnIndex >= 0 && columnIndex < columns.size());
	const CompositeColumn* column = columns.at(columnIndex);
	
	if (role == Qt::TextAlignmentRole) {
		return column->alignment;
	}
	
	int relevantRole = column->contentType == Bit ? Qt::CheckStateRole : Qt::DisplayRole;
	if (role != relevantRole) return QVariant();
	
	QVariant result = buffer.at(bufferRowIndex)->at(columnIndex);
	
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
	
	int previouslySelectedViewRowIndex = tableView->currentIndex().row();
	int previouslySelectedBufferRowIndex = getBufferRowIndexForViewRow(previouslySelectedViewRowIndex);
	
	if (allowPassAndReverse && column == currentSorting.first) {
		if (order == currentSorting.second) return;
		
		std::reverse(bufferOrder.begin(), bufferOrder.end());
	}
	else {
		auto comparator = [&column, order](int i1, int i2) {
			QVariant value1 = column->getRawValueAt(i1);
			QVariant value2 = column->getRawValueAt(i2);
			
			if (order == Qt::AscendingOrder) {
				return column->compare(value1, value2);
			} else {
				return column->compare(value2, value1);
			}
		};
		
		std::stable_sort(bufferOrder.begin(), bufferOrder.end(), comparator);
	}
	
	// Restore selection
	if (previouslySelectedBufferRowIndex >= 0) {
		int newViewRowIndex = findViewRowIndexForBufferRow(previouslySelectedBufferRowIndex);
		QModelIndex modelIndex = index(newViewRowIndex, 0);
		tableView->setCurrentIndex(modelIndex);
		tableView->scrollTo(modelIndex);
	}
	
	// Notify model users (views)
	QModelIndex topLeftIndex		= index(0, 0);
	QModelIndex bottomRightIndex	= index(bufferOrder.size() - 1, columns.size() - 1);
	Q_EMIT dataChanged(topLeftIndex, bottomRightIndex);
	//headerDataChanged(Qt::Vertical, 0, bufferOrder.size() - 1);
	
	currentSorting = {column, order};
}



QVariant CompositeTable::computeCellContent(int bufferRowIndex, int columnIndex) const
{
	assert(bufferRowIndex >= 0 && bufferRowIndex < baseTable->getNumberOfRows());
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
	assert(cells.size() == buffer.size());
	return cells;
}



ProjectSettings* CompositeTable::getProjectSettings() const
{
	return db->projectSettings;
}
