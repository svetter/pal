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
 * @file table_buffer.cpp
 * 
 * This file defines the TableBuffer and ViewOrderBuffer classes.
 */

#include "table_buffer.h"



/**
 * Creates an empty TableBuffer.
 */
TableBuffer::TableBuffer() :
	numColumns(0),
	buffer(QList<QList<QVariant>*>())
{}

/**
 * Destroys the TableBuffer.
 */
TableBuffer::~TableBuffer()
{
	qDeleteAll(buffer);
}


/**
 * Resets the TableBuffer to an empty state.
 */
void TableBuffer::reset()
{
	numColumns = 0;
	qDeleteAll(buffer);
	buffer.clear();
}

/**
 * Sets the initial number of columns for the buffer.
 * 
 * This method should only be called once, before any rows are added to the buffer.
 * 
 * @param initialNumColumns	The initial number of columns for the buffer.
 */
void TableBuffer::setInitialNumberOfColumns(int initialNumColumns)
{
	assert(numColumns < 1);
	assert(buffer.isEmpty());
	
	numColumns = initialNumColumns;
}



/**
 * Returns the number of rows in the buffer.
 * 
 * @return The number of rows in the buffer.
 */
int TableBuffer::numRows() const
{
	return buffer.size();
}

/**
 * Returns whether the buffer is empty.
 * 
 * @return True if the buffer contains no rows, false otherwise.
 */
bool TableBuffer::isEmpty() const
{
	return buffer.isEmpty();
}


/**
 * Returns the row at the given index.
 * 
 * @param rowIndex	The index of the row to return.
 * @return			The row at the given index.
 */
const QList<QVariant>* TableBuffer::getRow(BufferRowIndex rowIndex) const
{
	assert(rowIndex.isValid(numRows()));
	
	return buffer.at(rowIndex.get());
}

/**
 * Returns the value of the cell at the given index.
 * 
 * @param rowIndex		The row index of the cell to return.
 * @param columnIndex	The column index of the cell to return.
 * @return				The cell at the given location.
 */
QVariant TableBuffer::getCell(BufferRowIndex rowIndex, int columnIndex) const
{
	assert(columnIndex >= 0 && columnIndex < numColumns);
	
	const QList<QVariant>* row = getRow(rowIndex);
	return row->at(columnIndex);
}


/**
 * Appends a new row to the buffer.
 * 
 * @param newRow	The row to append.
 */
void TableBuffer::appendRow(QList<QVariant>* newRow)
{
	assert(newRow->size() == numColumns);
	
	buffer.append(newRow);
}

/**
 * Inserts a new row into the buffer at the given index.
 * 
 * @param rowIndex	The index at which to insert the new row.
 * @param newRow	The row to insert.
 */
void TableBuffer::insertRow(BufferRowIndex rowIndex, QList<QVariant>* newRow)
{
	assert(rowIndex.isValid(buffer.size() + 1));
	assert(newRow->size() == numColumns);
	
	buffer.insert(rowIndex.get(), newRow);
}

/**
 * Removes the row at the given index from the buffer.
 * 
 * @param rowIndex	The index of the row to remove.
 */
void TableBuffer::removeRow(BufferRowIndex rowIndex)
{
	assert(rowIndex.isValid(buffer.size()));
	
	const QList<QVariant>* rowToRemove = buffer.at(rowIndex.get());
	buffer.remove(rowIndex.get());
	delete rowToRemove;
}

/**
 * Replaces the value of the cell at the given index with the given value.
 * 
 * @param rowIndex		The row index of the cell to replace.
 * @param columnIndex	The column index of the cell to replace.
 * @param newValue		The new value for the cell.
 */
void TableBuffer::replaceCell(BufferRowIndex rowIndex, int columnIndex, const QVariant& newValue)
{
	assert(rowIndex.isValid(buffer.size()));
	assert(columnIndex >= 0 && columnIndex < numColumns);
	
	QList<QVariant>* row = buffer.at(rowIndex.get());
	row->replace(columnIndex, newValue);
}


/**
 * Appends space for a new column to the buffer.
 */
void TableBuffer::appendColumn()
{
	for (QList<QVariant>* const row : buffer) {
		row->append(QVariant());
	}
	
	numColumns++;
}

/**
 * Removes the column at the given index from the buffer.
 * 
 * @param columnIndex	The index of the column to remove.
 */
void TableBuffer::removeColumn(int columnIndex)
{
	assert(columnIndex >= 0 && columnIndex < numColumns);
	
	for (QList<QVariant>* const row : buffer) {
		row->remove(columnIndex);
	}
	
	numColumns--;
}



/**
 * Returns an iterator pointing to the first row in the buffer.
 * 
 * @return An iterator pointing to the first row in the buffer.
 */
QList<QList<QVariant>*>::const_iterator TableBuffer::begin() const
{
	return buffer.begin();
}

/**
 * Returns an iterator pointing to the row after the last row in the buffer.
 * 
 * @return An iterator pointing to the row after the last row in the buffer.
 */
QList<QList<QVariant>*>::const_iterator TableBuffer::end() const
{
	return buffer.end();
}





/**
 * Creates an empty ViewOrderBuffer.
 */
ViewOrderBuffer::ViewOrderBuffer() :
	order(QList<BufferRowIndex>())
{}


/**
 * Clears the ViewOrderBuffer.
 */
void ViewOrderBuffer::clear()
{
	order.clear();
}


/**
 * Returns the number of rows in the buffer.
 * 
 * @return The number of rows in the buffer.
 */
int ViewOrderBuffer::numRows() const
{
	return order.size();
}

/**
 * Returns whether the buffer is empty.
 * 
 * @return True if the buffer is empty, false otherwise.
 */
bool ViewOrderBuffer::isEmpty() const
{
	return order.isEmpty();
}


/**
 * Returns the buffer row index of the item at the given row in the view.
 * 
 * @param viewRowIndex	The index of the view row to look up.
 * @return				The buffer row index stored at the given view row index in the order buffer.
 */
BufferRowIndex ViewOrderBuffer::getBufferRowIndexForViewRow(ViewRowIndex viewRowIndex) const
{
	if (Q_UNLIKELY(viewRowIndex.isInvalid(order.size()))) return BufferRowIndex();
	return order.at(viewRowIndex.get());
}

/**
 * Determines at which row of the view the given buffer row index is stored, if any.
 * 
 * If the given buffer row index is not stored in the order buffer, returns an invalid
 * ViewRowIndex.
 * 
 * @param bufferRowIndex	The index of the buffer row to look up.
 * @return					The view row index where the given buffer row index is stored, if any.
 */
ViewRowIndex ViewOrderBuffer::findViewRowIndexForBufferRow(BufferRowIndex bufferRowIndex) const
{
	if (Q_UNLIKELY(bufferRowIndex.isInvalid())) return ViewRowIndex();
	return ViewRowIndex(order.indexOf(bufferRowIndex));
}


/**
 * Appends the given buffer row index to the order buffer.
 * 
 * @param rowIndex	The buffer row index to append.
 */
void ViewOrderBuffer::append(BufferRowIndex rowIndex)
{
	order.append(rowIndex);
}

/**
 * Inserts the given buffer row index into the order buffer at the given view row index.
 * 
 * @param viewRowIndex	The view row index at which to insert the given buffer row index.
 * @param rowIndex		The buffer row index to insert.
 */
void ViewOrderBuffer::removeViewRow(ViewRowIndex viewRowIndex)
{
	order.removeAt(viewRowIndex.get());
}

/**
 * Replaces the buffer row index stored at the given view row index.
 * 
 * @param viewRowIndex		The view row index at which to replace the buffer row index.
 * @param newBufferRowIndex	The new buffer row index to store at the given view row index.
 */
void ViewOrderBuffer::replaceBufferRowIndexAtViewRowIndex(ViewRowIndex viewRowIndex, BufferRowIndex newBufferRowIndex)
{
	order.replace(viewRowIndex.get(), newBufferRowIndex);
}


/**
 * Reverses the order of the buffer.
 */
void ViewOrderBuffer::reverse()
{
	std::reverse(order.begin(), order.end());
}

/**
 * Sorts the buffer according to the given comparator applied to the buffer row indices.
 * 
 * @param comparator	The comparator to use for sorting.
 */
void ViewOrderBuffer::sortBy(std::function<bool (const BufferRowIndex&, const BufferRowIndex&)> comparator)
{
	std::stable_sort(order.begin(), order.end(), comparator);
}
