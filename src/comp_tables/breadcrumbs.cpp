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
 * @file breadcrumbs.cpp
 * 
 * This file defines the Breadcrumb struct and the Breadcrumbs class.
 */

#include "breadcrumbs.h"

#include "src/db/table.h"



/**
 * Creates a Breadcrumb from two columns.
 * 
 * @param firstColumn	The first column in the breadcrumb pair.
 * @param secondColumn	The second column in the breadcrumb pair.
 */
Breadcrumb::Breadcrumb(Column* firstColumn, Column* secondColumn) :
	firstColumn(firstColumn),
	secondColumn(secondColumn)
{
	assert(firstColumn->table != secondColumn->table);
	assert(firstColumn->isForeignKey() != secondColumn->isForeignKey());
	if (isForward()) {
		assert(secondColumn->isPrimaryKey());
		assert(firstColumn->getReferencedForeignColumn() == secondColumn);
	} else {
		assert(firstColumn->isPrimaryKey());
		assert(secondColumn->getReferencedForeignColumn() == firstColumn);
	}
}

/**
 * Creates a forward-referencing Breadcrumb from a foreign key column and a primary key column.
 * 
 * @param firstColumn	The first column in the breadcrumb pair.
 * @param secondColumn	The second column in the breadcrumb pair.
 */
Breadcrumb::Breadcrumb(ForeignKeyColumn* firstColumn, PrimaryKeyColumn* secondColumn) :
	Breadcrumb((Column*) firstColumn, (Column*) secondColumn)
{}

/**
 * Creates a backward-referencing Breadcrumb from a primary key column and a foreign key column.
 * 
 * @param firstColumn	The first column in the breadcrumb pair.
 * @param secondColumn	The second column in the breadcrumb pair.
 */
Breadcrumb::Breadcrumb(PrimaryKeyColumn* firstColumn, ForeignKeyColumn* secondColumn) :
	Breadcrumb((Column*) firstColumn, (Column*) secondColumn)
{}



/**
 * Indicates whether the breadcrumb pair represents a forward reference.
 * 
 * A forward reference points from a foreign key column to the corresponding primary key column of
 * another table. When following a forward reference, the number of accumulated items can only stay
 * the same or decrease (because of overlap), never increase.
 * 
 * @return	True if the breadcrumb pair is a forward reference, false otherwise.
 */
bool Breadcrumb::isForward() const
{
	return firstColumn->isForeignKey();
}

/**
 * Indicates whether the breadcrumb pair represents a backward reference.
 * 
 * A backward reference points from a primary key column to a matching foreign key column of
 * another table. When following a backward reference, the number of accumulated items is only
 * bounded by the number of entries in the second table, meaning it can stay the same, decrease,
 * or increase.
 * 
 * @return	True if the breadcrumb pair is a forward reference, false otherwise.
 */
bool Breadcrumb::isBackward() const
{
	return secondColumn->isForeignKey();
}





/**
 * Creates a new Breadcrumbs list from an initializer list.
 * 
 * @param initList
 */
Breadcrumbs::Breadcrumbs(std::initializer_list<Breadcrumb> initList) :
	list(initList)
{
	assert(!list.isEmpty());
	assert(!list.first().firstColumn->table->isAssociative);
	
	const Table* currentTable = list.first().firstColumn->table;
	for (const auto& [firstColumn, secondColumn] : list) {
		assert(firstColumn->table == currentTable);
		currentTable = secondColumn->table;
	}
}


/**
 * Returns a set of all columns used as breadcrumbs.
 * 
 * @return	A set of all base table columns used as breadcrumbs.
 */
const QSet<Column* const> Breadcrumbs::getColumnSet() const
{
	QSet<Column* const> result = QSet<Column* const>();
	for (const auto& [firstColumn, secondColumn] : list) {
		result.insert(firstColumn);
		result.insert(secondColumn);
	}
	return result;
}


/**
 * @brief Evaluates the breadcrumb trail for a given row index.
 * 
 * The breadcrumb trail is evaluated from the given row index to the first breadcrumb.
 * The result is a set of row indices in the table of the first breadcrumb.
 * 
 * Following the breadcrumbs involves alternating between sets of row indices and primary keys (as
 * ItemID). The process starts with a single initial row index. From there, the first half of each
 * iteration turns a set of row indices into a set of keys. The second half of each iteration then
 * turns that set of keys into a new set of row indices.
 * 
 * This second half can work either in forward direction (lookup) or backward direction (reference
 * search). A forward lookup means looking for the row in a different table that matches each key in
 * the current key set. A backward reference search means looking for rows in a different table
 * where the key in the given column matches any key in the current key set.
 * 
 * The first column in each pair is used to collect a set of keys (as ItemID) by looking up the
 * value of that column for a set of buffer row indices. The second column in each pair is then used
 * to find rows in a different table that match the previously collected keys. After all breadcrumbs
 * have been followed, the content column is referenced to collect the values of the rows that were
 * found in the last breadcrumb.
 * 
 * @param initialBufferRowIndex	The row index to start from.
 * @return						The set of row indices in the table of the first breadcrumb.
 */
QSet<BufferRowIndex> Breadcrumbs::evaluate(BufferRowIndex initialBufferRowIndex) const
{
	QSet<BufferRowIndex> currentRowIndexSet = { initialBufferRowIndex };
	
	for (const Breadcrumb& crumb : list) {
		// Look up keys stored in firstColumn at given row indices
		QSet<ValidItemID> currentKeySet = QSet<ValidItemID>();
		for (const BufferRowIndex& bufferRowIndex : currentRowIndexSet) {
			ItemID key = crumb.firstColumn->getValueAt(bufferRowIndex);
			if (key.isInvalid()) continue;
			// Add new item ID to current set
			currentKeySet.insert(FORCE_VALID(key));
		}
		
		if (currentKeySet.isEmpty()) return QSet<BufferRowIndex>();
		
		currentRowIndexSet.clear();
		const Table* const table = crumb.secondColumn->table;
		
		// The second half of the transfer is dependent on the reference direction:
		if (crumb.isForward()) {
			// Forward reference (lookup, result for each input element is single key)
			// Find row matching each primary key
			for (const ValidItemID& key : currentKeySet) {
				BufferRowIndex bufferRowIndex = table->getMatchingBufferRowIndex({ crumb.secondColumn }, { key });
				// Add new buffer index to current set
				currentRowIndexSet.insert(bufferRowIndex);
			}
		}
		else {
			// Backward reference (reference search, result for each input element is key set)
			// Find rows in new table where key in secondColumn matches any key in current set
			for (const ValidItemID& key : currentKeySet) {
				const QList<BufferRowIndex> bufferRowIndexList = table->getMatchingBufferRowIndices(crumb.secondColumn, key.asQVariant());
				const QSet<BufferRowIndex> matchingBufferRowIndices = QSet<BufferRowIndex>(bufferRowIndexList.constBegin(), bufferRowIndexList.constEnd());
				// Add new buffer indices to current set
				currentRowIndexSet.unite(matchingBufferRowIndices);
			}
		}
		
		if (currentRowIndexSet.isEmpty()) return QSet<BufferRowIndex>();
	}
	
	return currentRowIndexSet;
}

QList<BufferRowIndex> Breadcrumbs::evaluateForStats(const QSet<BufferRowIndex>& initialBufferRowIndices) const
{
	QList<BufferRowIndex> currentRowIndexList = QList<BufferRowIndex>();
	for (const BufferRowIndex& bufferIndex : initialBufferRowIndices) {
		currentRowIndexList.append(bufferIndex);
	}
	
	for (const Breadcrumb& crumb : list) {
		// Look up keys stored in firstColumn at given row indices
		QSet<ValidItemID> currentKeySet = QSet<ValidItemID>();
		for (const BufferRowIndex& bufferRowIndex : currentRowIndexList) {
			ItemID key = crumb.firstColumn->getValueAt(bufferRowIndex);
			if (key.isInvalid()) continue;
			// Add new item ID to current set
			currentKeySet.insert(FORCE_VALID(key));
		}
		
		if (currentKeySet.isEmpty()) return QList<BufferRowIndex>();
		
		currentRowIndexList.clear();
		const Table* const table = crumb.secondColumn->table;
		
		// The second half of the transfer is dependent on the reference direction:
		if (crumb.isForward()) {
			// Forward reference (lookup, result for each input element is single key)
			// Find row matching each primary key
			for (const ValidItemID& key : currentKeySet) {
				BufferRowIndex bufferRowIndex = table->getMatchingBufferRowIndex({ crumb.secondColumn }, { key });
				// Add new buffer index to current set
				currentRowIndexList.append(bufferRowIndex);
			}
		}
		else {
			// Backward reference (reference search, result for each input element is key set)
			// Find rows in new table where key in secondColumn matches any key in current set
			for (const ValidItemID& key : currentKeySet) {
				const QList<BufferRowIndex> bufferRowIndexList = table->getMatchingBufferRowIndices(crumb.secondColumn, key.asQVariant());
				const QSet<BufferRowIndex> matchingBufferRowIndices = QSet<BufferRowIndex>(bufferRowIndexList.constBegin(), bufferRowIndexList.constEnd());
				// Add new buffer indices to current set
				for (const BufferRowIndex& bufferIndex : matchingBufferRowIndices) {
					currentRowIndexList.append(bufferIndex);
				}
			}
		}
		
		if (currentRowIndexList.isEmpty()) return QList<BufferRowIndex>();
	}
	
	return currentRowIndexList;
}
