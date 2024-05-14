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
#include "src/db/database.h"
#include "src/db/tables_spec/hikers_table.h"

#include <QCoreApplication>



/**
 * Constructs a new FoldCompositeColumn.
 *
 * @param type			The type of the composite column.
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param contentType	The type of data the column contents.
 * @param isStatistical	Whether the contents of this column display statistical data which can be excluded from exports.
 * @param suffix		A suffix to append to the content of each cell.
 * @param breadcrumbs	A list of column pairs that lead from the base table's primary key column to the content column.
 * @param contentColumn	The single column which contains the values to fold, or null if not present.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 */
FoldCompositeColumn::FoldCompositeColumn(CompColType type, CompositeTable& table, QString name, QString uiName, DataType contentType, bool isStatistical, QString suffix, const Breadcrumbs breadcrumbs, const ValueColumn* contentColumn, const QStringList* enumNames) :
	CompositeColumn(type, table, name, uiName, contentType, false, isStatistical, suffix, enumNames),
	breadcrumbs(breadcrumbs),
	contentColumn(contentColumn),
	contentTable(contentColumn ? (assert(!contentColumn->table.isAssociative), &((const NormalTable&) contentColumn->table)) : nullptr)
{
	if (contentColumn) {
		if (contentColumn->primaryKey) {
			assert(type == CountFold || type == ListStringFold);
		}
		assert(&contentColumn->table == &breadcrumbs.getTargetTable());
	}
}



/**
 * Returns a set of valid IDs for all rows in the target (content) table which are associated with
 * the given row of this composite column.
 * 
 * @param rowIndex	The row index for which to compute the associated target table IDs.
 * @return			A set of target table IDs associated with the given row.
 */
QSet<ValidItemID> FoldCompositeColumn::computeIDsAt(BufferRowIndex rowIndex) const
{
	const PrimaryKeyColumn& primaryKeyColumn = breadcrumbs.getTargetTable().primaryKeyColumn;
	const QSet<BufferRowIndex> targetRowIndexSet = breadcrumbs.evaluate(rowIndex);
	QSet<ValidItemID> ids = QSet<ValidItemID>();
	for (const BufferRowIndex& targetRowIndex : targetRowIndexSet) {
		ids.insert(VALID_ITEM_ID(primaryKeyColumn.getValueAt(targetRowIndex)));
	}
	return ids;
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
	if (contentColumn) {
		result.insert(contentColumn);
	}
	return result;
}





CountFoldCompositeColumn::CountFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const NormalTable& countTable) :
	FoldCompositeColumn(CountFold, table, name, uiName, Integer, true, suffix, table.crumbsTo(countTable), nullptr),
	countTable(countTable)
{}



QVariant CountFoldCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	const QSet<BufferRowIndex> targetRowIndexSet = breadcrumbs.evaluate(rowIndex);
	return targetRowIndexSet.size();
}



QStringList CountFoldCompositeColumn::encodeTypeSpecific() const
{
	QStringList parameters = QStringList();
	parameters += encodeString("countTable", countTable.name);
	parameters += encodeString("suffix", suffix);
	return parameters;
}

CountFoldCompositeColumn* CountFoldCompositeColumn::decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db)
{
	bool ok = false;
	
	const NormalTable* const countTable = decodeTableIdentity(restOfEncoding, "countTable", db, ok);
	if (!ok) return nullptr;
	
	const QString suffix = decodeString(restOfEncoding, "suffix", ok);
	if (!ok) return nullptr;
	
	return new CountFoldCompositeColumn(parentTable, name, uiName, suffix, *countTable);
}





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
NumericFoldCompositeColumn::NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, NumericFoldOp op, const ValueColumn& contentColumn) :
	FoldCompositeColumn(NumericFold, table, name, uiName, Integer, true, suffix, table.crumbsTo((assert(!contentColumn.table.isAssociative), (NormalTable&) contentColumn.table)), &contentColumn),
	op(op)
{
	assert(contentColumn.type == Integer);
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
	
	if (Q_UNLIKELY(rowIndexSet.isEmpty())) return QVariant();
	
	int aggregate = 0;
	if (op == MaxFold)	aggregate = INT_MIN;
	if (op == MinFold)	aggregate = INT_MAX;
	
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
			aggregate = std::max(aggregate, content.toInt());
			break;
		case MinFold:
			assert(content.canConvert<int>());
			aggregate = std::min(aggregate, content.toInt());
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
	case MinFold:		return aggregate;
	default:			assert(false);
	}
	return QVariant();
}



QStringList NumericFoldCompositeColumn::encodeTypeSpecific() const
{
	QStringList parameters = QStringList();
	parameters += encodeString("contentColumn_table_name", contentColumn->table.name);
	parameters += encodeString("contentColumn_name", contentColumn->name);
	parameters += encodeString("suffix", suffix);
	parameters += encodeString("foldOp", NumericFoldOpNames::getName(op));
	return parameters;
}

NumericFoldCompositeColumn* NumericFoldCompositeColumn::decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db)
{
	bool ok = false;
	
	const ValueColumn* const contentColumn = (const ValueColumn*) decodeColumnIdentity(restOfEncoding, "contentColumn_table_name", "contentColumn_name", db, ok);
	if (!ok) return nullptr;
	
	const QString suffix = decodeString(restOfEncoding, "suffix", ok);
	if (!ok) return nullptr;
	
	const QString opName = decodeString(restOfEncoding, "foldOp", ok);
	if (!ok) return nullptr;
	NumericFoldOp op = NumericFoldOpNames::getFoldOp(opName);
	if (op == NumericFoldOp(-1)) return nullptr;
	
	return new NumericFoldCompositeColumn(parentTable, name, uiName, suffix, op, *contentColumn);
}





/**
 * Constructs a new ListStringFoldCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param contentColumn	The column whose values to list.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 * @param isHikerList	Whether this is a HikerListFoldCompositeColumn.
 */
ListStringFoldCompositeColumn::ListStringFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, const ValueColumn& contentColumn, const QStringList* enumNames, bool isHikerList) :
	FoldCompositeColumn(isHikerList ? HikerListFold : ListStringFold, table, name, uiName, String, false, QString(), table.crumbsTo((assert(!contentColumn.table.isAssociative), (NormalTable&) contentColumn.table)), &contentColumn, enumNames)
{}



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
	
	const QList<QString> stringList = formatAndSortIntoStringList(rowIndexSet);
	return stringList.join(", ");
}

/**
 * As the first step of computing a cell value, formats cells with the given row indices from the
 * content column into a list of strings, then sorts the list alphabetically.
 */
QStringList ListStringFoldCompositeColumn::formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const
{
	QStringList stringList = QStringList();
	
	// Fetch and format to string
	for (const BufferRowIndex& rowIndex : rowIndexSet) {
		QVariant content;
		if (Q_UNLIKELY(contentColumn->primaryKey)) {
			content = contentTable->getIdentityRepresentationAt(rowIndex);
		} else if (Q_UNLIKELY(enumNames)) {
			content = replaceEnumIfApplicable(content);
		}
		else switch (contentColumn->type) {
		case Bit:	content = contentColumn->getValueAt(rowIndex).toBool() ? QCoreApplication::translate("FoldCompositeColumn", "Yes") : QCoreApplication::translate("FoldCompositeColumn", "No");	break;
		case Date:	content = contentColumn->getValueAt(rowIndex).toDate().toString(Qt::ISODate);	break;
		case Time:	content = contentColumn->getValueAt(rowIndex).toDate().toString(Qt::ISODate);	break;
		default:	content = contentColumn->getValueAt(rowIndex);
		}
		
		if (Q_UNLIKELY(!content.isValid() || content.toString().isEmpty())) continue;
		stringList.append(content.toString());
	}
	
	// Sort list of strings
	auto comparator = [] (const QString& string1, const QString& string2) {
		return QString::localeAwareCompare(string1, string2) < 0;
	};
	std::stable_sort(stringList.begin(), stringList.end(), comparator);
	
	return stringList;
}



QStringList ListStringFoldCompositeColumn::encodeTypeSpecific() const
{
	QStringList parameters = QStringList();
	parameters += encodeString("contentColumn_table_name", contentColumn->table.name);
	parameters += encodeString("contentColumn_name", contentColumn->name);
	return parameters;
}

ListStringFoldCompositeColumn* ListStringFoldCompositeColumn::decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db)
{
	bool ok = false;
	
	const ValueColumn* const contentColumn = (const ValueColumn*) decodeColumnIdentity(restOfEncoding, "contentColumn_table_name", "contentColumn_name", db, ok);
	if (!ok) return nullptr;
	
	const QStringList* enumNames = contentColumn->enumNames;
	
	return new ListStringFoldCompositeColumn(parentTable, name, uiName, *contentColumn, enumNames);
}





/**
 * Constructs a new HikerListCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param contentColumn	The hiker name column whose values to list.
 */
HikerListFoldCompositeColumn::HikerListFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, const ValueColumn& contentColumn) :
	ListStringFoldCompositeColumn(table, name, uiName, contentColumn, nullptr, true)
{
	assert(&contentColumn == &table.db.hikersTable.nameColumn);
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
	
	const QList<QString> stringList = formatAndSortIntoStringList(rowIndexSet);
	return stringList.join(", ");
}

/**
 * As the first step of computing a hiker list string, formats cells with the given row indices
 * from the content column into a list of strings, then sorts the list while keeping the default
 * hiker, if present, at the top of the list.
 */
QStringList HikerListFoldCompositeColumn::formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const
{
	QStringList stringList = QStringList();
	
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
