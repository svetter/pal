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
 * @file breadcrumbs.cpp
 * 
 * This file defines the Breadcrumb struct and the Breadcrumbs class.
 */

#include "breadcrumbs.h"

#include "src/db/normal_table.h"
#include "src/db/table.h"



/**
 * Creates a Breadcrumb from two columns.
 * 
 * @param firstColumn	The first column in the breadcrumb pair.
 * @param secondColumn	The second column in the breadcrumb pair.
 */
Breadcrumb::Breadcrumb(Column& firstColumn, Column& secondColumn) :
	firstColumn(firstColumn),
	secondColumn(secondColumn)
{
	assert(firstColumn.table != secondColumn.table);
	assert(firstColumn.isForeignKey() != secondColumn.isForeignKey());
	if (isForward()) {
		assert(secondColumn.isPrimaryKey());
		assert(firstColumn.getReferencedForeignColumn() == &secondColumn);
	} else {
		assert(firstColumn.isPrimaryKey());
		assert(secondColumn.getReferencedForeignColumn() == &firstColumn);
	}
}

/**
 * Creates a forward-referencing Breadcrumb from a foreign key column and a primary key column.
 * 
 * @param firstColumn	The first column in the breadcrumb pair.
 * @param secondColumn	The second column in the breadcrumb pair.
 */
Breadcrumb::Breadcrumb(ForeignKeyColumn& firstColumn, PrimaryKeyColumn& secondColumn) :
	Breadcrumb((Column&) firstColumn, (Column&) secondColumn)
{}

/**
 * Creates a backward-referencing Breadcrumb from a primary key column and a foreign key column.
 * 
 * @param firstColumn	The first column in the breadcrumb pair.
 * @param secondColumn	The second column in the breadcrumb pair.
 */
Breadcrumb::Breadcrumb(PrimaryKeyColumn& firstColumn, ForeignKeyColumn& secondColumn) :
	Breadcrumb((Column&) firstColumn, (Column&) secondColumn)
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
	return firstColumn.isForeignKey();
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
	return secondColumn.isForeignKey();
}


/**
 * Indicates whether the breadcrumb pair is equal to another breadcrumb pair.
 * 
 * @param other	The other breadcrumb pair to compare to.
 * @return		True if the breadcrumb pairs are equal, false otherwise.
 */
bool Breadcrumb::operator==(const Breadcrumb& other) const
{
	return &firstColumn == &other.firstColumn && &secondColumn == &other.secondColumn;
}

/**
 * Indicates whether the breadcrumb pair is not equal to another breadcrumb pair.
 * 
 * @param other	The other breadcrumb pair to compare to.
 * @return		True if the breadcrumb pairs are not equal, false otherwise.
 */
bool Breadcrumb::operator!=(const Breadcrumb& other) const
{
	return !operator==(other);
}





/**
 * Creates a new empty Breadcrumbs list.
 */
Breadcrumbs::Breadcrumbs() :
	list(QList<Breadcrumb>())
{}

/**
 * Creates a new Breadcrumbs list from a given list of breadcrumb pairs.
 *
 * @param breadcrumbs	The list to create the Breadcrumbs from.
 */
Breadcrumbs::Breadcrumbs(const QList<Breadcrumb>& breadcrumbs) :
	list(breadcrumbs)
{
	if (!list.isEmpty()) {
		assert(!list.first().firstColumn.table->isAssociative);
		assert(!list.last().secondColumn.table->isAssociative);
		
		const Table* previousTable = nullptr;
		const Table* currentTable = list.first().firstColumn.table;
		for (const auto& [firstColumn, secondColumn] : list) {
			assert(firstColumn.table == currentTable);
			assert(secondColumn.table != previousTable);
			
			previousTable = firstColumn.table;
			currentTable = secondColumn.table;
		}
	}
}

/**
 * Creates a new Breadcrumbs list from an initializer list.
 * 
 * @param initList	The initializer list to create the Breadcrumbs from.
 */
Breadcrumbs::Breadcrumbs(std::initializer_list<Breadcrumb> initList) :
	Breadcrumbs(QList<Breadcrumb>(initList))
{}


/**
 * Returns a set of all columns used as breadcrumbs.
 * 
 * @return	A set of all base table columns used as breadcrumbs.
 */
const QSet<Column*> Breadcrumbs::getColumnSet() const
{
	QSet<Column*> result = QSet<Column*>();
	for (const auto& [firstColumn, secondColumn] : list) {
		result.insert(&firstColumn);
		result.insert(&secondColumn);
	}
	return result;
}

/**
 * Returns the table which the first breadcrumb references with its first column.
 * 
 * @pre The list of breadcrumbs is not empty.
 * 
 * @return	The first table in the breadcrumb trail.
 */
const NormalTable* Breadcrumbs::getStartTable() const
{
	assert(!list.isEmpty());
	return (NormalTable*) list.first().firstColumn.table;
}

/**
 * Returns the table which the last breadcrumb references with its second column.
 * 
 * @pre The list of breadcrumbs is not empty.
 * 
 * @return	The last table in the breadcrumb trail.
 */
const NormalTable* Breadcrumbs::getTargetTable() const
{
	assert(!list.isEmpty());
	return (NormalTable*) list.last().secondColumn.table;
}


/**
 * Indicates whether the list of breadcrumbs is empty.
 * 
 * @return	True if the list of breadcrumbs is empty, false otherwise.
 */
bool Breadcrumbs::isEmpty() const
{
	return list.isEmpty();
}

/**
 * Returns the number of breadcrumb pairs in the list.
 * 
 * @return	The number of breadcrumb pairs in the list.
 */
int Breadcrumbs::length() const
{
	return list.size();
}

/**
 * Indicates whether the breadcrumb trail includes a given table.
 * 
 * @param table	The table to check for.
 * @return		True if the breadcrumb trail includes the given table, false otherwise.
 */
bool Breadcrumbs::goesVia(const Table& table) const
{
	for (int i = 1; i < list.size(); i++) {
		const Breadcrumb& crumb = list.at(i);
		if (crumb.firstColumn.table == &table) {
			return true;
		}
	}
	return false;
}


/**
 * Indicates whether the list of breadcrumbs is equal to another list of breadcrumbs.
 * 
 * @param other	The other list of breadcrumbs to compare to.
 * @return		True if the lists of breadcrumbs are equal, false otherwise.
 */
bool Breadcrumbs::operator==(const Breadcrumbs& other) const
{
	if (list.size() != other.list.size()) return false;
	for (int i = 0; i < list.size(); i++) {
		if (list.at(i) != other.list.at(i)) return false;
	}
	return true;
}

/**
 * Indicates whether the list of breadcrumbs is not equal to another list of breadcrumbs.
 * 
 * @param other	The other list of breadcrumbs to compare to.
 * @return		True if the lists of breadcrumbs are not equal, false otherwise.
 */
bool Breadcrumbs::operator!=(const Breadcrumbs& other) const
{
	return !operator==(other);
}


/**
 * Appends a new breadcrumb to the end of the list.
 * 
 * @param breadcrumb	The new breadcrumb to add to the list.
 */
void Breadcrumbs::append(const Breadcrumb& breadcrumb)
{
	if (list.isEmpty()) {
		assert(!breadcrumb.firstColumn.table->isAssociative);
	} else {
		assert(list.last().secondColumn.table == breadcrumb.firstColumn.table);
	}
	
	list.append(breadcrumb);
}


/**
 * Combines two breadcrumb lists into a new breadcrumb list.
 * 
 * @param other	The other breadcrumb list to combine with.
 * @return		The combined breadcrumb list.
 */
Breadcrumbs Breadcrumbs::operator+(const Breadcrumbs& other) const
{
	return Breadcrumbs(list + other.list);
}



/**
 * Evaluates the breadcrumb trail for a given row index.
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
			ItemID key = crumb.firstColumn.getValueAt(bufferRowIndex);
			if (key.isInvalid()) continue;
			// Add new item ID to current set
			currentKeySet.insert(FORCE_VALID(key));
		}
		
		if (currentKeySet.isEmpty()) return QSet<BufferRowIndex>();
		
		currentRowIndexSet.clear();
		const Table* const table = crumb.secondColumn.table;
		
		// The second half of the transfer is dependent on the reference direction:
		if (crumb.isForward()) {
			// Forward reference (lookup, result for each input element is single key)
			// Find row matching each primary key
			for (const ValidItemID& key : currentKeySet) {
				BufferRowIndex bufferRowIndex = table->getMatchingBufferRowIndex({ &crumb.secondColumn }, { key });
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

/**
 * Evaluates the breadcrumb trail, assuming it is entirely forward-referencing, and returns a single
 * row index, which may be invalid if a foreign key along the way was not set.
 * 
 * @param initialBufferRowIndex	The row index to start from.
 * @return						The corresponding row index in the target table.
 */
BufferRowIndex Breadcrumbs::evaluateAsForwardChain(BufferRowIndex initialBufferRowIndex) const
{
	BufferRowIndex currentRowIndex = initialBufferRowIndex;
	
	for (const Breadcrumb& crumb : list) {
		assert(crumb.isForward());
		
		// Look up key stored in current column at current row index
		const Column& currentColumn = crumb.firstColumn;
		const ItemID key = currentColumn.getValueAt(currentRowIndex);
		
		if (key.isInvalid()) return BufferRowIndex();
		
		// Get referenced primary key column of other table
		const PrimaryKeyColumn& referencedColumn = *currentColumn.getReferencedForeignColumn();
		assert(!referencedColumn.table->isAssociative);
		const NormalTable* currentTable = (NormalTable*) referencedColumn.table;
		
		// Find row index that contains the current primary key
		currentRowIndex = currentTable->getBufferIndexForPrimaryKey(FORCE_VALID(key));
		assert(currentRowIndex.isValid());
	}
	
	return currentRowIndex;
}

/**
 * Evaluates the breadcrumb trail, keeping count of duplicated entries along the way, which is
 * needed for calculating statistics.
 * 
 * This function is similar to evaluate(), but uses lists internally instead of sets. This allows
 * counting items multiple times instead of only counting the number of unique items. As an
 * exception, all duplicates are removed after an associative table has been traversed.
 * 
 * @param initialBufferRowIndices	The row indices to start from.
 * @return							The set of row indices in the table of the first breadcrumb.
 */
QList<BufferRowIndex> Breadcrumbs::evaluateForStats(const QSet<BufferRowIndex>& initialBufferRowIndices) const
{
	QList<BufferRowIndex> currentRowIndexList = QList<BufferRowIndex>();
	for (const BufferRowIndex& bufferIndex : initialBufferRowIndices) {
		currentRowIndexList.append(bufferIndex);
	}
	
	for (const Breadcrumb& crumb : list) {
		// Look up keys stored in firstColumn at given row indices
		QList<ValidItemID> currentKeyList = QList<ValidItemID>();
		for (const BufferRowIndex& bufferRowIndex : currentRowIndexList) {
			ItemID key = crumb.firstColumn.getValueAt(bufferRowIndex);
			if (key.isInvalid()) continue;
			// Add new item ID to current list
			currentKeyList.append(FORCE_VALID(key));
		}
		
		if (currentKeyList.isEmpty()) return QList<BufferRowIndex>();
		
		currentRowIndexList.clear();
		const Table* const table = crumb.secondColumn.table;
		
		// The second half of the transfer is dependent on the reference direction:
		if (crumb.isForward()) {
			// Forward reference (lookup, result for each input element is single key)
			// Find row matching each primary key
			for (const ValidItemID& key : currentKeyList) {
				BufferRowIndex bufferRowIndex = table->getMatchingBufferRowIndex({ &crumb.secondColumn }, { key });
				// Add new buffer index to current list
				currentRowIndexList.append(bufferRowIndex);
			}
		}
		else {
			// Backward reference (reference search, result for each input element is key list)
			// Find rows in new table where key in secondColumn matches any key in current list
			for (const ValidItemID& key : currentKeyList) {
				const QList<BufferRowIndex> matchingBufferRowIndices = table->getMatchingBufferRowIndices(crumb.secondColumn, key.asQVariant());
				// Add new buffer indices to current list
				currentRowIndexList.append(matchingBufferRowIndices);
			}
		}
		
		if (crumb.firstColumn.table->isAssociative && initialBufferRowIndices.size() > 1) {
			// Coming out of associative table => Remove all duplicates
			const QSet<BufferRowIndex> currentRowIndexSet = QSet<BufferRowIndex>(currentRowIndexList.constBegin(), currentRowIndexList.constEnd());
			currentRowIndexList.clear();
			currentRowIndexList = QList<BufferRowIndex>(currentRowIndexSet.constBegin(), currentRowIndexSet.constEnd());
		}
		
		if (currentRowIndexList.isEmpty()) return QList<BufferRowIndex>();
	}
	
	return currentRowIndexList;
}
