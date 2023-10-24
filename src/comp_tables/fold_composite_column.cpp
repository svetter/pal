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

#include "fold_composite_column.h"

#include "src/db/normal_table.h"
#include "src/db/tables/hikers_table.h"



FoldCompositeColumn::FoldCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, contentType, false, suffix, enumNames),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn)
{}





QSet<int> FoldCompositeColumn::evaluateBreadcrumbTrail(int rowIndex) const
{
	QSet<int> currentRowIndexSet = { rowIndex };
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
		for (int rowIndex : currentRowIndexSet) {
			ItemID key = firstColumn->getValueAt(rowIndex);
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
			for (ValidItemID key : currentKeySet) {
				int rowIndex = currentTable->getMatchingBufferRowIndex({ secondColumn }, { key });
				currentRowIndexSet.insert(rowIndex);
			}
		}
		else if (secondColumn->isForeignKey()) {
			// Backward reference (reference search, result for each input element is key set)
			assert(firstColumn == secondColumn->getReferencedForeignColumn());
			assert(firstColumn->isPrimaryKey());
			
			// Find rows in new currentTable where key in secondColumn matches any key in current set
			for (ValidItemID key : currentKeySet) {
				const QList<int> indexList = currentTable->getMatchingBufferRowIndices(secondColumn, key.asQVariant());
				const QSet<int> matchingBufferRowIndices = QSet<int>(indexList.constBegin(), indexList.constEnd());
				currentRowIndexSet.unite(matchingBufferRowIndices);
			}
		}
		else assert(false);
		
		if (currentRowIndexSet.isEmpty()) {
			return QSet<int>();
		}
	}
	
	return currentRowIndexSet;
}



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





NumericFoldCompositeColumn::NumericFoldCompositeColumn(CompositeTable* table, QString uiName, NumericFoldOp op, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn) :
		FoldCompositeColumn(table, uiName, Qt::AlignRight, op == CountFold ? Integer : op == IDListFold ? IDList : contentColumn->type, suffix, breadcrumbs, contentColumn),
		op(op)
{
	assert((op == CountFold) == (contentColumn == nullptr));
}

QVariant NumericFoldCompositeColumn::computeValueAt(int rowIndex) const
{
	QSet<int> rowIndexSet = evaluateBreadcrumbTrail(rowIndex);
	
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
		for (int rowIndex : rowIndexSet) {
			list.append(contentColumn->getValueAt(rowIndex));
		}
		return list;
	}
	
	// AVERAGE / SUM / MAX
	
	int aggregate = 0;
	
	for (int rowIndex : rowIndexSet) {
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





ListStringFoldCompositeColumn::ListStringFoldCompositeColumn(CompositeTable* table, QString uiName, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn) :
		FoldCompositeColumn(table, uiName, Qt::AlignLeft, String, QString(), breadcrumbs, contentColumn)
{}

QStringList ListStringFoldCompositeColumn::formatAndSortIntoStringList(QSet<int>& rowIndexSet) const
{
	QStringList stringList;
	
	for (int rowIndex : rowIndexSet) {
		QVariant content = contentColumn->getValueAt(rowIndex);
		assert(content.canConvert<QString>());
		content = replaceEnumIfApplicable(content);
		stringList.append(content.toString());
	}
	
	return stringList;
}

QVariant ListStringFoldCompositeColumn::computeValueAt(int rowIndex) const
{
	QSet<int> rowIndexSet = evaluateBreadcrumbTrail(rowIndex);
	
	QList<QString> stringList = formatAndSortIntoStringList(rowIndexSet);
	
	// Combine list into comma separated string
	QString listString = "";
	for (QString& string : stringList) {
		if (!listString.isEmpty()) listString.append(", ");
		listString.append(string);
	}
	return listString;
}





HikerListCompositeColumn::HikerListCompositeColumn(CompositeTable* table, QString uiName, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn) :
		ListStringFoldCompositeColumn(table, uiName, breadcrumbs, contentColumn)
{}

QStringList HikerListCompositeColumn::formatAndSortIntoStringList(QSet<int>& rowIndexSet) const
{
	QStringList stringList;
	
	QString defaultHikerString = QString();
	const ProjectSetting<int>* defaultHiker = getProjectSettings()->defaultHiker;
	const HikersTable* hikersTable = (HikersTable*) contentColumn->table;
	
	// Check whether default hiker is set and get name if so
	int defaultHikerRowIndex = hikersTable->getBufferIndexForPrimaryKey(defaultHiker->get());
	if (defaultHiker->isNotNull() && rowIndexSet.contains(defaultHikerRowIndex)) {
		QVariant content = contentColumn->getValueAt(defaultHikerRowIndex);
		assert(content.canConvert<QString>());
		defaultHikerString = content.toString();
		// Remove default hiker from row index set
		rowIndexSet.remove(defaultHikerRowIndex);
	}
	
	for (int rowIndex : rowIndexSet) {
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

QVariant HikerListCompositeColumn::computeValueAt(int rowIndex) const
{
	QSet<int> rowIndexSet = evaluateBreadcrumbTrail(rowIndex);
	
	QList<QString> stringList = formatAndSortIntoStringList(rowIndexSet);
	
	// Combine list into comma separated string
	QString listString = "";
	for (QString& string : stringList) {
		if (!listString.isEmpty()) listString.append(", ");
		listString.append(string);
	}
	return listString;
}