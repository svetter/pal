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

#include "src/db/tables/hikers_table.h"



/**
 * Creates a new Breadcrumb from two columns.
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
				currentRowIndexSet.insert(bufferRowIndex);
			}
		}
		else {
			// Backward reference (reference search, result for each input element is key set)
			// Find rows in new table where key in secondColumn matches any key in current set
			for (const ValidItemID& key : currentKeySet) {
				const QList<BufferRowIndex> bufferRowIndexList = table->getMatchingBufferRowIndices(crumb.secondColumn, key.asQVariant());
				const QSet<BufferRowIndex> matchingBufferRowIndices = QSet<BufferRowIndex>(bufferRowIndexList.constBegin(), bufferRowIndexList.constEnd());
				currentRowIndexSet.unite(matchingBufferRowIndices);
			}
		}
		
		if (currentRowIndexSet.isEmpty()) return QSet<BufferRowIndex>();
	}
	
	return currentRowIndexSet;
}





/**
 * Constructs a new FoldCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param contentType	The type of data the column contents.
 * @param isStatistical	Whether the contents of this column display statistical data which can be excluded from exports.
 * @param suffix		A suffix to append to the content of each cell.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 */
FoldCompositeColumn::FoldCompositeColumn(CompositeTable* table, QString name, QString uiName, DataType contentType, bool isStatistical, QString suffix, const Breadcrumbs breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, name, uiName, contentType, false, isStatistical, suffix, enumNames),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn)
{}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<Column* const> FoldCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> result = breadcrumbs.getColumnSet();
	if (contentColumn) result.insert(contentColumn);
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
NumericFoldCompositeColumn::NumericFoldCompositeColumn(CompositeTable* table, QString name, QString uiName, QString suffix, NumericFoldOp op, const Breadcrumbs breadcrumbs, Column* contentColumn) :
		FoldCompositeColumn(table, name, uiName, op == CountFold ? Integer : op == IDListFold ? IDList : contentColumn->type, true, suffix, breadcrumbs, contentColumn),
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
	QSet<BufferRowIndex> rowIndexSet = breadcrumbs.evaluate(rowIndex);
	
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
ListStringFoldCompositeColumn::ListStringFoldCompositeColumn(CompositeTable* table, QString name, QString uiName, const Breadcrumbs breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
		FoldCompositeColumn(table, name, uiName, String, false, QString(), breadcrumbs, contentColumn, enumNames)
{}

/**
 * As the first step of computing a cell value, formats cells with the given row indices from the
 * content column into a list of strings, then sorts the list alphabetically.
 */
QStringList ListStringFoldCompositeColumn::formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const
{
	QStringList stringList = QStringList();
	
	// Fetch and format to string
	for (const BufferRowIndex& rowIndex : rowIndexSet) {
		QVariant content = contentColumn->getValueAt(rowIndex);
		if (enumNames) {
			content = replaceEnumIfApplicable(content);
		} else {
			assert(content.canConvert<QString>());
		}
		stringList.append(content.toString());
	}
	
	// Sort list of strings
	auto comparator = [] (const QString& string1, const QString& string2) {
		return QString::localeAwareCompare(string1, string2) < 0;
	};
	std::stable_sort(stringList.begin(), stringList.end(), comparator);
	
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
	QSet<BufferRowIndex> rowIndexSet = breadcrumbs.evaluate(rowIndex);
	
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
HikerListCompositeColumn::HikerListCompositeColumn(CompositeTable* table, QString name, QString uiName, const Breadcrumbs breadcrumbs, Column* contentColumn) :
		ListStringFoldCompositeColumn(table, name, uiName, breadcrumbs, contentColumn)
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
	ProjectSetting<int>& defaultHiker = getProjectSettings()->defaultHiker;
	const HikersTable* hikersTable = (HikersTable*) contentColumn->table;
	
	// Check whether default hiker is set and get name if so
	if (defaultHiker.present()) {
		ValidItemID defaultHikerID = VALID_ITEM_ID(defaultHiker.get());
		BufferRowIndex defaultHikerRowIndex = hikersTable->getBufferIndexForPrimaryKey(defaultHikerID);
		if (rowIndexSet.contains(defaultHikerRowIndex)) {
			QVariant content = contentColumn->getValueAt(defaultHikerRowIndex);
			assert(content.canConvert<QString>());
			defaultHikerString = content.toString();
			// Remove default hiker from row index set
			rowIndexSet.remove(defaultHikerRowIndex);
		}
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
	QSet<BufferRowIndex> rowIndexSet = breadcrumbs.evaluate(rowIndex);
	
	QList<QString> stringList = formatAndSortIntoStringList(rowIndexSet);
	
	// Combine list into comma separated string
	QString listString = "";
	for (QString& string : stringList) {
		if (!listString.isEmpty()) listString.append(", ");
		listString.append(string);
	}
	return listString;
}
