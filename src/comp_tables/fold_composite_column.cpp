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
 * @file fold_composite_column.cpp
 * 
 * This file defines the FoldCompositeColumn class and its subclasses.
 */

#include "fold_composite_column.h"

#include "src/comp_tables/composite_table.h"
#include "src/db/tables_spec/hikers_table.h"



/**
 * Constructs a new FoldCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param contentType	The type of data the column contents.
 * @param isStatistical	Whether the contents of this column display statistical data which can be excluded from exports.
 * @param suffix		A suffix to append to the content of each cell.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 */
FoldCompositeColumn::FoldCompositeColumn(CompositeTable& table, QString name, QString uiName, DataType contentType, bool isStatistical, QString suffix, const Breadcrumbs breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
	CompositeColumn(table, name, uiName, contentType, false, isStatistical, suffix, enumNames),
	breadcrumbs(breadcrumbs),
	contentColumn(contentColumn)
{
	assert(!contentColumn || &contentColumn->table == &breadcrumbs.getTargetTable());
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> FoldCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<const Column*> result = breadcrumbs.getColumnSet();
	if (contentColumn) result.insert(contentColumn);
	return result;
}





/**
 * Constructs a new NumericFoldCompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param op			The operation to perform on the content column values.
 * @param contentType	The type of data the column contents.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param contentColumn	The column whose values to count, collect, or fold.
 */
NumericFoldCompositeColumn::NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, NumericFoldOp op, DataType contentType, const Breadcrumbs breadcrumbs, Column* contentColumn) :
	FoldCompositeColumn(table, name, uiName, contentType, true, suffix, breadcrumbs, contentColumn),
	op(op)
{
	assert((op == CountFold) == (contentColumn == nullptr));
}

/**
 * Constructs a new NumericFoldCompositeColumn with FoldOp CountFold.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param op			The operation to perform on the content column values.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 */
NumericFoldCompositeColumn::NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, NumericFoldOp op, const Breadcrumbs breadcrumbs) :
	NumericFoldCompositeColumn(table, name, uiName, suffix, op, Integer, breadcrumbs, nullptr)
{}

/**
 * Constructs a new NumericFoldCompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param op			The operation to perform on the content column values.
 * @param contentColumn	The column whose values to count, collect, or fold.
 */
NumericFoldCompositeColumn::NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, NumericFoldOp op, Column& contentColumn) :
	NumericFoldCompositeColumn(table, name, uiName, suffix, op, contentColumn.type, table.crumbsTo((assert(!contentColumn.table.isAssociative), (NormalTable&) contentColumn.table)), &contentColumn)
{}

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
		case AverageFold:
		case SumFold:
		case MaxFold:
			return QVariant();
		default: assert(false);
		}
	}
	
	// COUNT
	
	if (op == CountFold) {
		return rowIndexSet.size();
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
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param contentColumn	The column whose values to list.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 */
ListStringFoldCompositeColumn::ListStringFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, Column& contentColumn, const QStringList* enumNames) :
	FoldCompositeColumn(table, name, uiName, String, false, QString(), table.crumbsTo((assert(!contentColumn.table.isAssociative), (NormalTable&) contentColumn.table)), &contentColumn, enumNames)
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
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param contentColumn	The hiker name column whose values to list.
 */
HikerListFoldCompositeColumn::HikerListFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, ValueColumn& contentColumn) :
	ListStringFoldCompositeColumn(table, name, uiName, contentColumn)
{}

/**
 * As the first step of computing a hiker list string, formats cells with the given row indices
 * from the content column into a list of strings, then sorts the list while keeping the default
 * hiker, if present, at the top of the list.
 */
QStringList HikerListFoldCompositeColumn::formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const
{
	QStringList stringList;
	
	QString defaultHikerString = QString();
	const ProjectSetting<int>& defaultHiker = getProjectSettings().defaultHiker;
	const HikersTable& hikersTable = (HikersTable&) contentColumn->table;
	
	// Check whether default hiker is set and get name if so
	if (defaultHiker.present()) {
		ValidItemID defaultHikerID = VALID_ITEM_ID(defaultHiker.get());
		BufferRowIndex defaultHikerRowIndex = hikersTable.getBufferIndexForPrimaryKey(defaultHikerID);
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
QVariant HikerListFoldCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
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
