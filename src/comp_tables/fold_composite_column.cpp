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
 * @file fold_composite_column.cpp
 * 
 * This file defines the FoldCompositeColumn class and its subclasses.
 */

#include "fold_composite_column.h"

#include "src/db/normal_table.h"
#include "src/db/tables/hikers_table.h"



/**
 * Constructs a new FoldCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param alignment		The alignment of the column contents.
 * @param contentType	The type of data the column contents.
 * @param suffix		A suffix to append to the content of each cell.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 */
FoldCompositeColumn::FoldCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, contentType, false, suffix, enumNames),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn)
{}





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
 * search). A forward lookup means looking for the row in a different table that matches each key
 * in the current key set. A backward reference search means looking for rows in a different table
 * where the key in the given column matches any key in the current key set.
 * 
 * The first column in each pair is used to collect a set of keys (as ItemID) by looking up the
 * value of that column for a set of buffer row indices. The second column in each pair is then
 * used to find rows in a different table that match the previously collected keys. After all
 * breadcrumbs have been followed, the content column is referenced to collect the values of the
 * rows that were found in the last breadcrumb.
 * 
 * @param initialBufferRowIndex The row index to start from.
 * @return The set of row indices in the table of the first breadcrumb.
 */
QSet<BufferRowIndex> FoldCompositeColumn::evaluateBreadcrumbTrail(BufferRowIndex initialBufferRowIndex) const
{
	QSet<BufferRowIndex> currentRowIndexSet = { initialBufferRowIndex };
	const Table* currentTable = (NormalTable*) breadcrumbs.first().first->table;
	
	for (int round = 0; round < breadcrumbs.size(); round++) {
		const Column* firstColumn	= breadcrumbs.at(round).first;
		const Column* secondColumn	= breadcrumbs.at(round).second;
		
		// Check continuity
		assert(firstColumn->table == currentTable);
		assert(firstColumn->table != secondColumn->table);
		assert(firstColumn->isForeignKey() != secondColumn->isForeignKey());
		
		// Look up keys stored in firstColumn at given row indices
		QSet<ValidItemID> currentKeySet = QSet<ValidItemID>();
		for (const BufferRowIndex& bufferRowIndex : currentRowIndexSet) {
			ItemID key = firstColumn->getValueAt(bufferRowIndex);
			if (key.isValid()) currentKeySet.insert(key.forceValid());
		}
		
		currentRowIndexSet.clear();
		currentTable = secondColumn->table;
		
		// The second half of the transfer is dependent on the reference direction:
		if (firstColumn->isForeignKey()) {
			// Forward reference (lookup, result for each input element is single key)
			assert(firstColumn->getReferencedForeignColumn() == secondColumn);
			assert(secondColumn->isPrimaryKey());
			
			// Find row matching each primary key
			for (const ValidItemID& key : currentKeySet) {
				BufferRowIndex bufferRowIndex = currentTable->getMatchingBufferRowIndex({ secondColumn }, { key });
				currentRowIndexSet.insert(bufferRowIndex);
			}
		}
		else if (secondColumn->isForeignKey()) {
			// Backward reference (reference search, result for each input element is key set)
			assert(firstColumn == secondColumn->getReferencedForeignColumn());
			assert(firstColumn->isPrimaryKey());
			
			// Find rows in new currentTable where key in secondColumn matches any key in current set
			for (const ValidItemID& key : currentKeySet) {
				const QList<BufferRowIndex> bufferRowIndexList = currentTable->getMatchingBufferRowIndices(secondColumn, key.asQVariant());
				const QSet<BufferRowIndex> matchingBufferRowIndices = QSet<BufferRowIndex>(bufferRowIndexList.constBegin(), bufferRowIndexList.constEnd());
				currentRowIndexSet.unite(matchingBufferRowIndices);
			}
		}
		else assert(false);
		
		if (currentRowIndexSet.isEmpty()) {
			return QSet<BufferRowIndex>();
		}
	}
	
	return currentRowIndexSet;
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<Column* const> FoldCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> result = QSet<Column* const>();
	if (contentColumn) result.insert(contentColumn);
	for (QPair<Column*, Column*> pair : breadcrumbs) {
		result.insert(pair.first);
		result.insert(pair.second);
	}
	return result;
}





/**
 * Constructs a new NumericFoldCompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param op			The operation to perform on the content column values.
 * @param suffix		A suffix to append to the content of each cell.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param contentColumn	The column whose values to count, collect, or fold.
 */
NumericFoldCompositeColumn::NumericFoldCompositeColumn(CompositeTable* table, QString uiName, NumericFoldOp op, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn) :
		FoldCompositeColumn(table, uiName, Qt::AlignRight, op == CountFold ? Integer : op == IDListFold ? IDList : contentColumn->type, suffix, breadcrumbs, contentColumn),
		op(op)
{
	assert((op == CountFold) == (contentColumn == nullptr));
}

/**
 * Computes the value of the cell at the given row index.
 * 
 * Delegates breadcrumb evaluation to FoldCompositeColumn::evaluateBreadcrumbTrail().
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant NumericFoldCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QSet<BufferRowIndex> rowIndexSet = evaluateBreadcrumbTrail(rowIndex);
	
	// Shortcuts for empty set
	if (rowIndexSet.isEmpty()) {
		switch (op) {
		case CountFold:
			return 0;
		case IDListFold:
			return QList<QVariant>();
		case AverageFold:
		case SumFold:
		case MaxFold:
			return QVariant();
		default: assert(false);
		}
	}
	
	// COUNT / ID LIST
	
	if (op == CountFold) {
		return rowIndexSet.size();
	}
	if (op == IDListFold) {
		QList<QVariant> list = QList<QVariant>();
		for (const BufferRowIndex& rowIndex : rowIndexSet) {
			list.append(contentColumn->getValueAt(rowIndex));
		}
		return list;
	}
	
	// AVERAGE / SUM / MAX
	
	int aggregate = 0;
	
	for (const BufferRowIndex& rowIndex : rowIndexSet) {
		QVariant content = contentColumn->getValueAt(rowIndex);
		
		switch (op) {
		case AverageFold:
		case SumFold:
			assert(content.canConvert<int>());
			aggregate += content.toInt();
			break;
		case MaxFold:
			assert(content.canConvert<int>());
			if (content.toInt() > aggregate) aggregate = content.toInt();
			break;
		default:
			assert(false);
		}
	}
	
	assert(!rowIndexSet.isEmpty());
	
	switch (op) {
	case AverageFold:	return std::round((qreal) aggregate / rowIndexSet.size());
	case SumFold:		return aggregate;
	case MaxFold:		return aggregate;
	default:			assert(false);
	}
	return QVariant();
}





/**
 * Constructs a new ListStringFoldCompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param contentColumn	The column whose values to list.
 */
ListStringFoldCompositeColumn::ListStringFoldCompositeColumn(CompositeTable* table, QString uiName, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
		FoldCompositeColumn(table, uiName, Qt::AlignLeft, String, QString(), breadcrumbs, contentColumn, enumNames)
{}

/**
 * As the first step of computing a cell value, formats cells with the given row indices from the
 * content column into a list of strings, then sorts the list alphabetically.
 */
QStringList ListStringFoldCompositeColumn::formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const
{
	QStringList stringList;
	
	for (const BufferRowIndex& rowIndex : rowIndexSet) {
		QVariant content = contentColumn->getValueAt(rowIndex);
		assert(content.canConvert<QString>());
		content = replaceEnumIfApplicable(content);
		stringList.append(content.toString());
	}
	
	return stringList;
}

/**
 * Computes the value of the cell at the given row index.
 * 
 * Delegates breadcrumb evaluation to FoldCompositeColumn::evaluateBreadcrumbTrail() and string
 * formatting and sorting to formatAndSortIntoStringList().
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant ListStringFoldCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QSet<BufferRowIndex> rowIndexSet = evaluateBreadcrumbTrail(rowIndex);
	
	QList<QString> stringList = formatAndSortIntoStringList(rowIndexSet);
	
	// Combine list into comma separated string
	QString listString = "";
	for (QString& string : stringList) {
		if (!listString.isEmpty()) listString.append(", ");
		listString.append(string);
	}
	return listString;
}





/**
 * Constructs a new HikerListCompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param contentColumn	The hiker name column whose values to list.
 */
HikerListCompositeColumn::HikerListCompositeColumn(CompositeTable* table, QString uiName, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn) :
		ListStringFoldCompositeColumn(table, uiName, breadcrumbs, contentColumn)
{}

/**
 * As the first step of computing a hiker list string, formats cells with the given row indices
 * from the content column into a list of strings, then sorts the list while keeping the default
 * hiker, if present, at the top of the list.
 */
QStringList HikerListCompositeColumn::formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const
{
	QStringList stringList;
	
	QString defaultHikerString = QString();
	const ProjectSetting<int>* defaultHiker = getProjectSettings()->defaultHiker;
	const HikersTable* hikersTable = (HikersTable*) contentColumn->table;
	
	// Check whether default hiker is set and get name if so
	BufferRowIndex defaultHikerRowIndex = hikersTable->getBufferIndexForPrimaryKey(defaultHiker->get());
	if (defaultHiker->isNotNull() && rowIndexSet.contains(defaultHikerRowIndex)) {
		QVariant content = contentColumn->getValueAt(defaultHikerRowIndex);
		assert(content.canConvert<QString>());
		defaultHikerString = content.toString();
		// Remove default hiker from row index set
		rowIndexSet.remove(defaultHikerRowIndex);
	}
	
	for (const BufferRowIndex& rowIndex : rowIndexSet) {
		QVariant content = contentColumn->getValueAt(rowIndex);
		assert(content.canConvert<QString>());
		stringList.append(content.toString());
	}
	std::sort(stringList.begin(), stringList.end());
	
	if (!defaultHikerString.isEmpty()) {
		stringList.insert(0, defaultHikerString);
	}
	
	return stringList;
}

/**
 * Computes the value of the cell at the given row index.
 * 
 * Delegates breadcrumb evaluation to FoldCompositeColumn::evaluateBreadcrumbTrail() and string
 * formatting and sorting to formatAndSortIntoStringList().
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant HikerListCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QSet<BufferRowIndex> rowIndexSet = evaluateBreadcrumbTrail(rowIndex);
	
	QList<QString> stringList = formatAndSortIntoStringList(rowIndexSet);
	
	// Combine list into comma separated string
	QString listString = "";
	for (QString& string : stringList) {
		if (!listString.isEmpty()) listString.append(", ");
		listString.append(string);
	}
	return listString;
}
