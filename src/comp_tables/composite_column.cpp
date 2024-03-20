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
 * @file composite_column.cpp
 * 
 * This file defines the CompositeColumn class and some of its subclasses.
 */

#include "composite_column.h"

#include "src/comp_tables/composite_table.h"
#include "src/data/enum_names.h"



/**
 * Creates a CompositeColumn.
 * 
 * @param table						The CompositeTable that this column belongs to.
 * @param name						The internal name for this column.
 * @param uiName					The name of this column as it should be displayed in the UI.
 * @param contentType				The type of data the column contents.
 * @param cellsAreInterdependent	Whether the contents of the cells in this column depend on each other.
 * @param isStatistical				Whether the column is a statistical column.
 * @param suffix					A suffix to append to the content of each cell.
 * @param enumNames					An optional list of enum names with which to replace the raw cell content.
 * @param enumNameLists				An optional list of enum name lists with which to replace the raw cell content.
 */
CompositeColumn::CompositeColumn(CompositeTable& table, QString name, QString uiName, DataType contentType, bool cellsAreInterdependent, bool isStatistical, QString suffix, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
	table(table),
	name(name),
	uiName(uiName),
	alignment((contentType == Integer || contentType == ID) ? Qt::AlignRight : contentType == Bit ? Qt::AlignCenter : Qt::AlignLeft),
	contentType(contentType),
	cellsAreInterdependent(cellsAreInterdependent),
	isStatistical(isStatistical),
	enumNames(enumNames),
	enumNameLists(enumNameLists),
	suffix(suffix)
{
	assert(!cellsAreInterdependent || isStatistical);
}

/**
 * Destroys the CompositeColumn.
 */
CompositeColumn::~CompositeColumn()
{}



/**
 * Formats the raw content of a cell for display in the UI.
 * 
 * Formats integers according to locale, dates and time, replaces enumerative indices with their
 * corresponding strings, and appends the suffix.
 * 
 * @param rawCellContent	The raw cell content.
 * @return					The cell content formatted to string.
 */
QString CompositeColumn::toFormattedTableContent(QVariant rawCellContent) const
{
	if (!rawCellContent.isValid()) return QString();
	
	QString result = rawCellContent.toString();
	
	if (contentType == Integer) {
		assert(rawCellContent.canConvert<int>());
		int number = rawCellContent.toInt();
		if (abs(number) > 9999) {
			result = QLocale().toString(number);
		}
	}
	
	else if (contentType == Date) {
		assert(rawCellContent.canConvert<QDate>());
		result = rawCellContent.toDate().toString("dd.MM.yyyy");
	}
	
	else if (contentType == Time) {
		assert(rawCellContent.canConvert<QTime>());
		result = rawCellContent.toTime().toString("HH:mm");
	}
	
	else if (contentType == Enum) {
		assert(enumNames);
		assert(rawCellContent.canConvert<int>());
		int enumIndex = rawCellContent.toInt();
		if (!(enumIndex >= 0 && enumIndex < enumNames->size()))
		assert(enumIndex >= 0 && enumIndex < enumNames->size());
		result = EnumNames::tr(enumNames->at(enumIndex).toStdString().c_str());
	}
	else if (contentType == DualEnum) {
		assert(enumNameLists);
		assert(rawCellContent.canConvert<QList<QVariant>>());
		QList<QVariant> intList = rawCellContent.toList();
		assert(intList.size() == 2);
		assert(intList.at(0).canConvert<int>() && intList.at(1).canConvert<int>());
		const int discerningEnumIndex	= intList.at(0).toInt();
		const int displayedEnumIndex	= intList.at(1).toInt();
		assert(discerningEnumIndex >= 0 && discerningEnumIndex <= enumNameLists->size());
		QStringList specifiedEnumNames = EnumNames::translateList(enumNameLists->at(discerningEnumIndex).second);
		specifiedEnumNames.removeAt(1);
		assert(displayedEnumIndex >= 0 && displayedEnumIndex <= specifiedEnumNames.size());
		result = specifiedEnumNames.at(displayedEnumIndex);
	}
	
	return result + suffix;
}



/**
 * Returns the column's index in its table.
 * 
 * @pre This is a normal or filter-only column, not an export-only column.
 * 
 * @return	The column's index.
 */
int CompositeColumn::getIndex() const
{
	return table.getIndexOf(*this);
}

/**
 * Returns the column's index in the export-only column list of its table.
 * 
 * @pre This is an export-only column, not a normal or filter-only column.
 * 
 * @return	The column's index in the export-only column list.
 */
int CompositeColumn::getExportIndex() const
{
	return table.getExportIndexOf(*this);
}

/**
 * Indicates whether this column is only used for exports and not for UI display.
 * 
 * @return	True if this is an export-only column, false otherwise.
 */
bool CompositeColumn::isExportOnlyColumn() const
{
	return table.getExportIndexOf(*this) >= 0;
}

/**
 * Indicates whether this column is only used for filtering and not for UI display.
 * 
 * @return	True if this is a filter-only column, false otherwise.
 */
bool CompositeColumn::isFilterOnlyColumn() const
{
	return getIndex() >= table.getNumberOfNormalColumns();
}



/**
 * Computes the value of all cells in the column together.
 *
 * This is used for columns with interdependent cells, such as the IndexCompositeColumn.
 *
 * @return	Computed values for all cells in the column.
 */
QList<QVariant> CompositeColumn::computeWholeColumn() const
{
	QList<QVariant> cells = QList<QVariant>();
	for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(table.getBaseTable().getNumberOfRows()); rowIndex++) {
		cells.append(computeValueAt(rowIndex));
	}
	return cells;
}



/**
 * Returns the raw computed value of the cell at the given row index.
 * 
 * @param rowIndex	The row index.
 * @return			The raw computed value of the cell.
 */
QVariant CompositeColumn::getRawValueAt(BufferRowIndex rowIndex) const
{
	return table.getRawValue(rowIndex, *this);
}

/**
 * Returns the formatted computed value of the cell at the given row index.
 * 
 * @param rowIndex	The row index.
 * @return			The formatted computed value of the cell.
 */
QVariant CompositeColumn::getFormattedValueAt(BufferRowIndex rowIndex) const
{
	return table.getFormattedValue(rowIndex, *this);
}



/**
 * Replaced enumerative indices with their corresponding strings in columns which have enumNames
 * specified.
 * 
 * @param content	The raw cell content.
 * @return			The enum string corresponding to the index in the raw cell content.
 */
QVariant CompositeColumn::replaceEnumIfApplicable(QVariant content) const
{
	if (!enumNames) return content;
	
	// If content is enumerative, replace index with the corresponding string
	assert(content.canConvert<int>());
	int index = content.toInt();
	assert(index >= 0 && index < enumNames->size());
	return EnumNames::tr(enumNames->at(index).toStdString().c_str());
}



/**
 * Compares two cells of this column.
 * 
 * @param value1	The first cell's value.
 * @param value2	The second cell's value.
 * @return			True if the first cell's value is smaller than the second cell's value.
 */
bool CompositeColumn::compare(const QVariant& value1, const QVariant& value2) const
{
	return compareCells(contentType, value1, value2);
}



/**
 * Applies a filter to a given list of rows in this column.
 * 
 * @param filter		The filter to apply.
 * @param orderBuffer	The list of rows to apply the filter to.
 */
void CompositeColumn::applySingleFilter(const Filter& filter, ViewOrderBuffer& orderBuffer) const
{
	if (orderBuffer.isEmpty()) return;
	
	std::function<bool (QVariant)> valuePasses = nullptr;
	// TODO
	
	switch (contentType) {
	case Integer: {
		
		break;
	}
	case ID: {
		
		break;
	}
	case IDList: {
		
		break;
	}
	case Enum: {
		
		break;
	}
	case DualEnum: {
		
		break;
	}
	case Bit: {
		
		break;
	}
	case String: {
		
		break;
	}
	case Date: {
		
		break;
	}
	case Time: {
		
		break;
	}
	default: assert(false);
	}
	
	if (!valuePasses) return;
	// Do the actual filter pass
	for (ViewRowIndex viewRow = ViewRowIndex(orderBuffer.numRows() - 1); viewRow.isValid(); viewRow--) {
		BufferRowIndex bufferRow = orderBuffer.getBufferRowIndexForViewRow(viewRow);
		bool letThrough = valuePasses(getRawValueAt(bufferRow));
		if (letThrough) continue;
		orderBuffer.removeViewRow(viewRow);
	}
}



/**
 * Returns the project settings of the table this column belongs to.
 * 
 * @return	The project settings.
 */
const ProjectSettings& CompositeColumn::getProjectSettings() const
{
	return table.getProjectSettings();
}





/**
 * Creates a DirectCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param suffix		A suffix to append to the content of each cell.
 * @param contentColumn	The column from which to take the actual cell content.
 */
DirectCompositeColumn::DirectCompositeColumn(CompositeTable& table, QString suffix, Column& contentColumn) :
	CompositeColumn(table, contentColumn.name, contentColumn.uiName, contentColumn.type, false, false, suffix, contentColumn.enumNames),
	contentColumn(contentColumn)
{}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DirectCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	return contentColumn.getValueAt(rowIndex);
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> DirectCompositeColumn::getAllUnderlyingColumns() const
{
	return { &contentColumn };
}





/**
 * Creates a ReferenceCompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param contentColumn	The column from which to take the actual cell content.
 */
ReferenceCompositeColumn::ReferenceCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, Column& contentColumn) :
	CompositeColumn(table, name, uiName, contentColumn.type, false, false, suffix, contentColumn.enumNames),
	breadcrumbs((assert(!contentColumn.table.isAssociative), table.crumbsTo((NormalTable&) contentColumn.table))),
	contentColumn(contentColumn)
{
	assert(&contentColumn.table == &breadcrumbs.getTargetTable());
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant ReferenceCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	BufferRowIndex targetRowIndex = breadcrumbs.evaluateAsForwardChain(rowIndex);
	
	if (Q_UNLIKELY(targetRowIndex.isInvalid())) return QVariant();
	
	// Look up content column at last row index
	QVariant content = contentColumn.getValueAt(targetRowIndex);
	
	return content;
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> ReferenceCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<const Column*> result = { &contentColumn };
	result.unite(breadcrumbs.getColumnSet());
	return result;
}





/**
 * Creates a DifferenceCompositeColumn.
 *
 * @param table				The CompositeTable that this column belongs to.
 * @param name				The internal name for this column.
 * @param uiName			The name of this column as it should be displayed in the UI.
 * @param suffix			A suffix to append to the content of each cell.
 * @param minuendColumn		The column from which to take the minuends.
 * @param subtrahendColumn	The column from which to take the subtrahends.
 */
DifferenceCompositeColumn::DifferenceCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, ValueColumn& minuendColumn, ValueColumn& subtrahendColumn) :
	CompositeColumn(table, name, uiName, Integer, false, true, suffix),
	minuendColumn(minuendColumn),
	subtrahendColumn(subtrahendColumn)
{
	assert(&minuendColumn.table == &subtrahendColumn.table);
	assert(!minuendColumn.isKey() && !subtrahendColumn.isKey());
	assert(minuendColumn.type == subtrahendColumn.type);
	assert(&minuendColumn != &subtrahendColumn);
	assert(minuendColumn.type == Integer || minuendColumn.type == Date);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DifferenceCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QVariant minuendContent = minuendColumn.getValueAt(rowIndex);
	QVariant subtrahendContent = subtrahendColumn.getValueAt(rowIndex);
	
	if (Q_UNLIKELY(!minuendContent.isValid() || !subtrahendContent.isValid())) return QVariant();
	
	switch (minuendColumn.type) {
	case Integer: {
		assert(minuendContent.canConvert<int>() && subtrahendContent.canConvert<int>());
		int minuend = minuendContent.toInt();
		int subtrahend = subtrahendContent.toInt();
		
		return minuend - subtrahend;
	}
	case Date: {
		assert(minuendContent.canConvert<QDate>() && subtrahendContent.canConvert<QDate>());
		QDate minuend = minuendContent.toDate();
		QDate subtrahend = subtrahendContent.toDate();
		
		return subtrahend.daysTo(minuend) + 1;
	}
	default:
		assert(false);
		return QVariant();
	}
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> DifferenceCompositeColumn::getAllUnderlyingColumns() const
{
	return { &minuendColumn, &subtrahendColumn };
}





/**
 * Creates a DependentEnumCompositeColumn.
 *
 * @param table					The CompositeTable that this column belongs to.
 * @param name					The internal name for this column.
 * @param uiName				The name of this column as it should be displayed in the UI.
 * @param discerningEnumColumn	The column from which to take the discerning enum.
 * @param displayedEnumColumn	The column from which to take the displayed enum.
 */
DependentEnumCompositeColumn::DependentEnumCompositeColumn(CompositeTable& table, QString name, QString uiName, ValueColumn& discerningEnumColumn, ValueColumn& displayedEnumColumn) :
	CompositeColumn(table, name, uiName, DualEnum, false, false, QString(), nullptr, discerningEnumColumn.enumNameLists),
	discerningEnumColumn(discerningEnumColumn),
	displayedEnumColumn(displayedEnumColumn)
{
	assert(&discerningEnumColumn.table == &displayedEnumColumn.table);
	assert(!discerningEnumColumn.isKey() && !displayedEnumColumn.isKey());
	assert(discerningEnumColumn.type == DualEnum && displayedEnumColumn.type == DualEnum);
	assert(discerningEnumColumn.enumNameLists == displayedEnumColumn.enumNameLists);
	assert(&discerningEnumColumn != &displayedEnumColumn);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DependentEnumCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QVariant discerningContent = discerningEnumColumn.getValueAt(rowIndex);
	QVariant displayedContent = displayedEnumColumn.getValueAt(rowIndex);
	
	assert(discerningContent.canConvert<int>() && displayedContent.canConvert<int>());
	int discerning = discerningContent.toInt();
	int displayed = displayedContent.toInt();
	
	if (Q_UNLIKELY(discerning < 1 || displayed < 1)) return QVariant();
	
	return QVariant(QList<QVariant>({ discerning, displayed }));
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> DependentEnumCompositeColumn::getAllUnderlyingColumns() const
{
	return { &discerningEnumColumn, &displayedEnumColumn };
}





/**
 * Creates a CompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param sortingPasses	The list of sorting passes in order of priority, each containing column and order.
 */
IndexCompositeColumn::IndexCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const QList<BaseSortingPass> sortingPasses) :
	CompositeColumn(table, name, uiName, Integer, true, true, suffix),
	sortingPasses(sortingPasses)
{
	assert(!sortingPasses.isEmpty());
	for (const auto& [column, order] : sortingPasses) {
		assert(&column.table == &table.getBaseTable());
		assert(order == Qt::AscendingOrder || order == Qt::DescendingOrder);
	}
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant IndexCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	qDebug() << "CAUTION: Using extremely inefficient IndexCompositeColumn::computeValueAt(BufferRowIndex)";
	QList<BufferRowIndex> order = getRowIndexOrderList();
	return order.indexOf(rowIndex) + 1;
}

/**
 * Computes the value of all cells in the column together.
 *
 * This is used for columns with interdependent cells, such as the IndexCompositeColumn.
 *
 * @return	Computed values for all cells in the column.
 */
QList<QVariant> IndexCompositeColumn::computeWholeColumn() const
{
	QList<BufferRowIndex> order = getRowIndexOrderList();
	
	QList<QVariant> cells = QList<QVariant>();
	for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(order.size()); rowIndex++) {
		cells.append(order.indexOf(rowIndex) + 1);
	}
	
	return cells;
}

QList<BufferRowIndex> IndexCompositeColumn::getRowIndexOrderList() const
{
	int numberOfRows = sortingPasses.at(0).column.table.getNumberOfRows();
	// Local order buffer which represents the ordered list of row indices
	QList<BufferRowIndex> order = QList<BufferRowIndex>();
	for (BufferRowIndex index = BufferRowIndex(0); index.isValid(numberOfRows); index++) {
		order.append(index);
	}
	
	for (int i = sortingPasses.size() - 1; i >= 0; i--) {
		const BaseSortingPass sorting = sortingPasses.at(i);
		
		auto comparator = [&sorting](BufferRowIndex i1, BufferRowIndex i2) {
			QVariant value1 = sorting.column.getValueAt(i1);
			QVariant value2 = sorting.column.getValueAt(i2);
			
			if (sorting.order == Qt::AscendingOrder) {
				return compareCells(sorting.column.type, value1, value2);
			} else {
				return compareCells(sorting.column.type, value2, value1);
			}
		};
		
		std::stable_sort(order.begin(), order.end(), comparator);
	}
	
	return order;
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> IndexCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<const Column*> columns = QSet<const Column*>();
	for (const auto& [column, order] : sortingPasses) {
		columns += &column;
	}
	return columns;
}





/**
 * Creates a CompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param sortingPasses	The list of columns to sort by and their sort order, in order of priority. The first column automatically doubles as the separating (grouping) column.
 */
OrdinalCompositeColumn::OrdinalCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const QList<BaseSortingPass> sortingPasses) :
	IndexCompositeColumn(table, name, uiName, suffix, sortingPasses),
	separatingColumn(sortingPasses.first().column)
{
	assert(sortingPasses.first().column.isForeignKey());
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant OrdinalCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	qDebug() << "CAUTION: Using extremely inefficient OrdinalCompositeColumn::computeValueAt(BufferRowIndex)";
	return computeWholeColumn().at(rowIndex.get());
}

/**
 * Computes the value of all cells in the column together.
 *
 * This is used for columns with interdependent cells, such as the IndexCompositeColumn.
 *
 * @return	Computed values for all cells in the column.
 */
QList<QVariant> OrdinalCompositeColumn::computeWholeColumn() const
{
	QList<BufferRowIndex> order = getRowIndexOrderList();
	
	QList<QVariant> ordinals = QList<QVariant>(order.size());
	
	ItemID lastKey = ItemID();
	int ordinal = 1;
	for (const BufferRowIndex& rowIndex : order) {
		ItemID currentKey = separatingColumn.getValueAt(rowIndex);
		if (Q_UNLIKELY(!currentKey.isValid())) {
			// No key, reset ordinal and append empty
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex.get(), QVariant());
		} else if (Q_LIKELY(currentKey != lastKey)) {
			// Next key, reset ordinal
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex.get(), ordinal);
		} else {
			// Same key, increase ordinal
			ordinal++;
			ordinals.replace(rowIndex.get(), ordinal);
		}
	}
	
	return ordinals;
}
