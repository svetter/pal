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

#include "table_buffer.h"



TableBuffer::TableBuffer() : buffer(QList<QList<QVariant>*>())
{}

TableBuffer::~TableBuffer()
{
	qDeleteAll(buffer);
}


void TableBuffer::reset()
{
	qDeleteAll(buffer);
	buffer.clear();
}


int TableBuffer::numRows() const
{
	return buffer.size();
}

bool TableBuffer::isEmpty() const
{
	return buffer.isEmpty();
}


const QList<QVariant>* TableBuffer::getRow(BufferRowIndex rowIndex) const
{
	assert(rowIndex.isValid(numRows()));
	return buffer.at(rowIndex.get());
}

QVariant TableBuffer::getCell(BufferRowIndex rowIndex, int columnIndex) const
{
	const QList<QVariant>* row = getRow(rowIndex);
	assert(columnIndex >= 0 && columnIndex < row->size());
	return row->at(columnIndex);
}


void TableBuffer::appendRow(QList<QVariant>* newRow)
{
	buffer.append(newRow);
}

void TableBuffer::insertRow(BufferRowIndex rowIndex, QList<QVariant>* newRow)
{
	buffer.insert(rowIndex.get(), newRow);
}

void TableBuffer::removeRow(BufferRowIndex rowIndex)
{
	assert(rowIndex.isValid(buffer.size()));
	const QList<QVariant>* rowToRemove = buffer.at(rowIndex.get());
	buffer.remove(rowIndex.get());
	delete rowToRemove;
}

void TableBuffer::replaceCell(BufferRowIndex rowIndex, int columnIndex, const QVariant& newValue)
{
	assert(rowIndex.isValid(buffer.size()));
	QList<QVariant>* row = buffer.at(rowIndex.get());
	assert(columnIndex >= 0 && columnIndex < row->size());
	row->replace(columnIndex, newValue);
}


QList<QList<QVariant>*>::const_iterator TableBuffer::begin() const
{
	return buffer.begin();
}
QList<QList<QVariant>*>::const_iterator TableBuffer::end() const
{
	return buffer.end();
}





ViewOrderBuffer::ViewOrderBuffer() : order(QList<BufferRowIndex>())
{}


void ViewOrderBuffer::clear()
{
	order.clear();
}


int ViewOrderBuffer::numRows() const
{
	return order.size();
}

bool ViewOrderBuffer::isEmpty() const
{
	return order.isEmpty();
}


BufferRowIndex ViewOrderBuffer::getBufferRowIndexForViewRow(ViewRowIndex viewRowIndex) const
{
	if (viewRowIndex.isInvalid(order.size())) return BufferRowIndex();
	return order.at(viewRowIndex.get());
}

ViewRowIndex ViewOrderBuffer::findViewRowIndexForBufferRow(BufferRowIndex bufferRowIndex) const
{
	if (bufferRowIndex.isInvalid()) return ViewRowIndex();
	return ViewRowIndex(order.indexOf(bufferRowIndex));
}


void ViewOrderBuffer::append(BufferRowIndex rowIndex)
{
	order.append(rowIndex);
}

void ViewOrderBuffer::removeViewRow(ViewRowIndex viewRowIndex)
{
	order.removeAt(viewRowIndex.get());
}

void ViewOrderBuffer::replaceBufferRowIndexAtViewRowIndex(ViewRowIndex viewRowIndex, BufferRowIndex newBufferRowIndex)
{
	order.replace(viewRowIndex.get(), newBufferRowIndex);
}


void ViewOrderBuffer::reverse()
{
	std::reverse(order.begin(), order.end());
}

void ViewOrderBuffer::sortBy(std::function<bool (const BufferRowIndex&, const BufferRowIndex&)> comparator)
{
	std::stable_sort(order.begin(), order.end(), comparator);
}
