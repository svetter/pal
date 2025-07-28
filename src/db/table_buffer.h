/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file table_buffer.h
 * 
 * This file declares the TableBuffer and ViewOrderBuffer classes.
 */

#ifndef TABLE_BUFFER_H
#define TABLE_BUFFER_H

#include "qvariant.h"
#include "src/db/row_index.h"

#include "QList"



/**
 * A class encapsulating a buffer for a Table or a CompositeTable.
 */
class TableBuffer {
protected:
	/** The number of columns currently represented in each row of the buffer. */
	int numColumns;
	/** The buffer. */
	QList<QList<QVariant>*> buffer;
	
public:
	TableBuffer();
	~TableBuffer();
	
	void reset();
	void setInitialNumberOfColumns(int initialNumColumns);
	
	int numRows() const;
	bool isEmpty() const;
	
	const QList<QVariant>* getRow(BufferRowIndex rowIndex) const;
	QVariant getCell(BufferRowIndex rowIndex, int columnIndex) const;
	
	void appendRow(QList<QVariant>* newRow);
	void insertRow(BufferRowIndex rowIndex, QList<QVariant>* newRow);
	void removeRow(BufferRowIndex rowIndex);
	void replaceCell(BufferRowIndex rowIndex, int columnIndex, const QVariant& newValue);
	
	void appendColumn();
	void removeColumn(int columnIndex);
	
	QList<QList<QVariant>*>::const_iterator begin() const;
	QList<QList<QVariant>*>::const_iterator end() const;
};





/**
 * A class encapsulating an order buffer (a list of ViewOrderBuffer) for a CompositeTable.
 */
class ViewOrderBuffer {
protected:
	/** The order buffer. */
	QList<BufferRowIndex> order;
	
public:
	ViewOrderBuffer();
	
	void clear();
	
	int numRows() const;
	bool isEmpty() const;
	
	BufferRowIndex getBufferRowIndexForViewRow(ViewRowIndex viewRowIndex) const;
	ViewRowIndex findViewRowIndexForBufferRow(BufferRowIndex bufferRowIndex) const;
	
	void append(BufferRowIndex rowIndex);
	void removeViewRow(ViewRowIndex viewRowIndex);
	void replaceBufferRowIndexAtViewRowIndex(ViewRowIndex viewRowIndex, BufferRowIndex newBufferRowIndex);
	
	void reverse();
	void sortBy(std::function<bool (const BufferRowIndex&, const BufferRowIndex&)> comparator);
};



#endif // TABLE_BUFFER_H
