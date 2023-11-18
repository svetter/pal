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
 * @param uiName					The name of this column as it should be displayed in the UI.
 * @param alignment					The alignment of the column contents.
 * @param contentType				The type of data the column contents.
 * @param cellsAreInterdependent	Whether the contents of the cells in this column depend on each other.
 * @param isStatistical				Whether the column is a statistical column.
 * @param suffix					A suffix to append to the content of each cell.
 * @param enumNames					An optional list of enum names with which to replace the raw cell content.
 * @param enumNameLists				An optional list of enum name lists with which to replace the raw cell content.
 */
CompositeColumn::CompositeColumn(CompositeTable* table, QString name, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, bool cellsAreInterdependent, bool isStatistical, QString suffix, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
		table(table),
		name(name),
		uiName(uiName),
		alignment(alignment),
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
		int discerningEnumIndex	= intList.at(0).toInt();
		int displayedEnumIndex	= intList.at(1).toInt();
		assert(discerningEnumIndex >= 0 && discerningEnumIndex <= enumNameLists->size());
		QList<QString> specifiedEnumNames = EnumNames::translateList(enumNameLists->at(discerningEnumIndex).second);
		assert(displayedEnumIndex >= 0 && displayedEnumIndex <= specifiedEnumNames.size());
		result = specifiedEnumNames.at(displayedEnumIndex);
	}
	
	return result + suffix;
}



/**
 * Returns the column's index in its table.
 * 
 * @return	The column's index.
 */
int CompositeColumn::getIndex() const
{
	return table->getIndexOf(this);
}

/**
 * Indicates whether this column is for backend purposes only and not for UI display.
 * 
 * @return	True if this is a backend column, false otherwise.
 */
bool CompositeColumn::isBackendColumn() const
{
	return getIndex() >= table->getNumberOfVisibleColumns();
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
	for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(table->getBaseTable()->getNumberOfRows()); rowIndex++) {
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
	return table->getRawValue(rowIndex, this);
}

/**
 * Returns the formatted computed value of the cell at the given row index.
 * 
 * @param rowIndex	The row index.
 * @return			The formatted computed value of the cell.
 */
QVariant CompositeColumn::getFormattedValueAt(BufferRowIndex rowIndex) const
{
	return table->getFormattedValue(rowIndex, this);
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
	assert(filter.column == this);
	
	const QVariant value		= filter.value;
	const bool hasSecond		= filter.hasSecond;
	const QVariant secondValue	= filter.secondValue;
	
	const bool isInt	= value.canConvert<int>()	&& (!hasSecond || secondValue.canConvert<int>());
	const bool isBool	= value.canConvert<bool>()	&& (!hasSecond || secondValue.canConvert<bool>());
	const bool isDate	= value.canConvert<QDate>()	&& (!hasSecond || secondValue.canConvert<QDate>());
	const bool isTime	= value.canConvert<QTime>()	&& (!hasSecond || secondValue.canConvert<QTime>());
	
	std::function<bool (QVariant)> valuePasses = nullptr;
	
	switch (contentType) {
	case Integer: {
		if (!isInt) { qDebug() << "Problem with parsing filter: Column content type is Integer but filter values are not"; return; }
		int value1 = value.toInt();
		if (hasSecond) {
			int value2 = secondValue.toInt();
			valuePasses = [value1, value2] (QVariant valueToFilter) {
				return value1 <= valueToFilter.toInt() && valueToFilter.toInt() <= value2;
			};
		} else {
			valuePasses = [value1] (QVariant valueToFilter) {
				return valueToFilter.toInt() == value1;
			};
		}
		break;
	}
	case ID: {
		ItemID id = isInt ? value.toInt() : ItemID();
		valuePasses = [id] (QVariant valueToFilter) {
			if (id.isInvalid()) return ItemID(valueToFilter).isInvalid();
			return valueToFilter == id.asQVariant();
		};
		break;
	}
	case IDList: {
		ItemID id = isInt ? value.toInt() : ItemID();
		valuePasses = [id] (QVariant valueToFilter) {
			QList<QVariant> list = valueToFilter.toList();
			if (id.isInvalid()) return list.isEmpty();
			return list.contains(id.asQVariant());
		};
		break;
	}
	case Enum: {
		if (!isInt) { qDebug() << "Problem with parsing filter: Column content type is Enum but filter value is not int"; return; }
		int intValue = value.toInt();
		if (intValue < 0) { qDebug() << "Problem with parsing filter: Column content type is Integer but filter value is invalid as an enum"; return; }
		valuePasses = [intValue] (QVariant valueToFilter) {
			return valueToFilter.toInt() == intValue;
		};
		break;
	}
	case DualEnum: {
		if (!hasSecond) { qDebug() << "Problem with parsing filter: Column content type is DualEnum but filter only has one value"; return; }
		if (!isInt) { qDebug() << "Problem with parsing filter: Column content type is DualEnum but filter value list value is not int"; return; }
		int discerning = value.toInt();
		if (discerning < 0) { qDebug() << "Problem with parsing filter: Column content type is DualEnum but filter value list value is invalid as an enum"; return; }
		int displayed = secondValue.toInt();
		if (displayed < 0) { qDebug() << "Problem with parsing filter: Column content type is DualEnum but filter value list value is invalid as an enum"; return; }
		valuePasses = [discerning, displayed] (QVariant valueToFilter) {
			QList<QVariant> list = valueToFilter.toList();
			if (list.size() < 2) return discerning == 0;
			return list.at(0) == discerning && list.at(1) == displayed;
		};
		break;
	}
	case Bit: {
		if (!isBool) { qDebug() << "Problem with parsing filter: Column content type is Bit but filter value is not bool"; return; }
		bool boolValue = value.toBool();
		valuePasses = [boolValue] (QVariant valueToFilter) {
			return valueToFilter.toBool() == boolValue;
		};
		break;
	}
	case String: {
		qDebug() << "WARNING: Case String in CompositeColumn::applySingleFilter() should probably not be used";
		QString stringValue = value.toString();
		valuePasses = [stringValue] (QVariant valueToFilter) {
			return valueToFilter.toString() == stringValue;
		};
		break;
	}
	case Date: {
		if (!isDate) { qDebug() << "Problem with parsing filter: Column content type is Date but filter values are not QDate"; return; }
		QDate value1 = value.toDate();
		if (!value1.isValid()) { qDebug() << "Problem with parsing filter: Column content type is Date but filter value is invalid as QDate"; return; }
		if (hasSecond) {
			QDate value2 = secondValue.toDate();
			if (!value2.isValid()) { qDebug() << "Problem with parsing filter: Column content type is Date but filter value is invalid as QDate"; return; }
			valuePasses = [value1, value2] (QVariant valueToFilter) {
				return value1 <= valueToFilter.toDate() && valueToFilter.toDate() <= value2;
			};
		} else {
			valuePasses = [value1] (QVariant valueToFilter) {
				return valueToFilter.toDate() == value1;
			};
		}
		break;
	}
	case Time: {
		if (!isTime) { qDebug() << "Problem with parsing filter: Column content type is Time but filter values are not QTime"; return; }
		QTime value1 = value.toTime();
		if (!value1.isValid()) { qDebug() << "Problem with parsing filter: Column content type is Time but filter value is invalid as QTime"; return; }
		if (hasSecond) {
			QTime value2 = secondValue.toTime();
			if (!value2.isValid()) { qDebug() << "Problem with parsing filter: Column content type is Time but filter value list value is invalid as QTime"; return; }
			valuePasses = [value1, value2] (QVariant valueToFilter) {
				return value1 <= valueToFilter.toTime() && valueToFilter.toTime() <= value2;
			};
		} else {
			valuePasses = [value1] (QVariant valueToFilter) {
				return valueToFilter.toTime() == value1;
			};
		}
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
 * This function can be called to announce that some data from which this column is computed has
 * changed, and the column needs to be updated.
 */
void CompositeColumn::announceChangedData() const
{
	int thisColumnIndex = table->getIndexOf(this);
	table->announceChangesUnderColumn(thisColumnIndex);
}



/**
 * Returns the project settings of the table this column belongs to.
 * 
 * @return	The project settings.
 */
ProjectSettings* CompositeColumn::getProjectSettings() const
{
	return table->getProjectSettings();
}





/**
 * Creates a DirectCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param alignment		The alignment of the column contents.
 * @param suffix		A suffix to append to the content of each cell.
 * @param contentColumn	The column from which to take the actual cell content.
 */
DirectCompositeColumn::DirectCompositeColumn(CompositeTable* table, QString name, QString uiName, Qt::AlignmentFlag alignment, QString suffix, Column* contentColumn) :
		CompositeColumn(table, name, uiName, alignment, contentColumn->type, false, false, suffix, contentColumn->enumNames),
		contentColumn(contentColumn)
{
	assert(contentColumn);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DirectCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	return contentColumn->getValueAt(rowIndex);
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<Column* const> DirectCompositeColumn::getAllUnderlyingColumns() const
{
	return { contentColumn };
}





/**
 * Creates a ReferenceCompositeColumn.
 *
 * @param table						The CompositeTable that this column belongs to.
 * @param uiName					The name of this column as it should be displayed in the UI.
 * @param alignment					The alignment of the column contents.
 * @param suffix					A suffix to append to the content of each cell.
 * @param foreignKeyColumnSequence	The sequence of foreign key columns to follow to get to the content column's table.
 * @param contentColumn				The column from which to take the actual cell content.
 */
ReferenceCompositeColumn::ReferenceCompositeColumn(CompositeTable* table, QString name, QString uiName, Qt::AlignmentFlag alignment, QString suffix, QList<Column*> foreignKeyColumnSequence, Column* contentColumn) :
		CompositeColumn(table, name, uiName, alignment, contentColumn->type, false, false, suffix, contentColumn->enumNames),
		foreignKeyColumnSequence(foreignKeyColumnSequence),
		contentColumn(contentColumn)
{
	assert(contentColumn);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant ReferenceCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	assert(foreignKeyColumnSequence.first()->isForeignKey());
	
	BufferRowIndex currentRowIndex = rowIndex;
	assert(!foreignKeyColumnSequence.first()->table->isAssociative);
	NormalTable* currentTable = (NormalTable*) foreignKeyColumnSequence.first()->table;
	
	for (const Column* currentColumn : foreignKeyColumnSequence) {
		assert(currentColumn->table == currentTable);
		assert(currentColumn->isForeignKey());
		
		// Look up key stored in current column at current row index
		ItemID key = currentColumn->getValueAt(currentRowIndex);
		
		if (key.isInvalid()) return QVariant();
		
		// Get referenced primary key column of other table
		const Column* referencedColumn = currentColumn->getReferencedForeignColumn();
		assert(!referencedColumn->table->isAssociative);
		currentTable = (NormalTable*) referencedColumn->table;
		
		// Find row index that contains the current primary key
		currentRowIndex = currentTable->getBufferIndexForPrimaryKey(FORCE_VALID(key));
		assert(currentRowIndex.isValid());
	}
	
	// Finally, look up content column at last row index
	assert(contentColumn->table == currentTable);
	QVariant content = contentColumn->getValueAt(currentRowIndex);
	
	return content;
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<Column* const> ReferenceCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> result = { contentColumn };
	for (Column* column : foreignKeyColumnSequence) {
		result.insert(column);
		result.insert(column->getReferencedForeignColumn());
	}
	return result;
}





/**
 * Creates a DifferenceCompositeColumn.
 *
 * @param table				The CompositeTable that this column belongs to.
 * @param uiName			The name of this column as it should be displayed in the UI.
 * @param suffix			A suffix to append to the content of each cell.
 * @param minuendColumn		The column from which to take the minuends.
 * @param subtrahendColumn	The column from which to take the subtrahends.
 */
DifferenceCompositeColumn::DifferenceCompositeColumn(CompositeTable* table, QString name, QString uiName, QString suffix, Column* minuendColumn, Column* subtrahendColumn) :
		CompositeColumn(table, name, uiName, Qt::AlignRight, Integer, false, true, suffix),
		minuendColumn(minuendColumn),
		subtrahendColumn(subtrahendColumn)
{
	assert(minuendColumn && subtrahendColumn);
	assert(minuendColumn->table == subtrahendColumn->table);
	assert(!minuendColumn->isKey() && !subtrahendColumn->isKey());
	assert(minuendColumn->type == subtrahendColumn->type);
	assert(minuendColumn != subtrahendColumn);
	assert(minuendColumn->type == Integer || minuendColumn->type == Date);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DifferenceCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QVariant minuendContent = minuendColumn->getValueAt(rowIndex);
	QVariant subtrahendContent = subtrahendColumn->getValueAt(rowIndex);
	
	if (!minuendContent.isValid() || !subtrahendContent.isValid()) return QVariant();
	
	switch (minuendColumn->type) {
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
const QSet<Column* const> DifferenceCompositeColumn::getAllUnderlyingColumns() const
{
	return { minuendColumn, subtrahendColumn };
}





/**
 * Creates a DependentEnumCompositeColumn.
 *
 * @param table					The CompositeTable that this column belongs to.
 * @param uiName				The name of this column as it should be displayed in the UI.
 * @param discerningEnumColumn	The column from which to take the discerning enum.
 * @param displayedEnumColumn	The column from which to take the displayed enum.
 * @param enumNameLists			An optional list of enum name lists with which to replace the raw cell content.
 */
DependentEnumCompositeColumn::DependentEnumCompositeColumn(CompositeTable* table, QString name, QString uiName, Column* discerningEnumColumn, Column* displayedEnumColumn) :
		CompositeColumn(table, name, uiName, Qt::AlignLeft, DualEnum, false, false, QString(), nullptr, discerningEnumColumn->enumNameLists),
		discerningEnumColumn(discerningEnumColumn),
		displayedEnumColumn(displayedEnumColumn)
{
	assert(discerningEnumColumn && displayedEnumColumn);
	assert(discerningEnumColumn->table == displayedEnumColumn->table);
	assert(!discerningEnumColumn->isKey() && !displayedEnumColumn->isKey());
	assert(discerningEnumColumn->type == DualEnum && displayedEnumColumn->type == DualEnum);
	assert(discerningEnumColumn->enumNameLists == displayedEnumColumn->enumNameLists);
	assert(discerningEnumColumn != displayedEnumColumn);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DependentEnumCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QVariant discerningContent = discerningEnumColumn->getValueAt(rowIndex);
	QVariant displayedContent = displayedEnumColumn->getValueAt(rowIndex);
	
	assert(discerningContent.canConvert<int>() && displayedContent.canConvert<int>());
	int discerning = discerningContent.toInt();
	int displayed = displayedContent.toInt();
	
	if (discerning < 1 || displayed < 1) return QVariant();
	
	return QVariant(QList<QVariant>({ discerning, displayed }));
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<Column* const> DependentEnumCompositeColumn::getAllUnderlyingColumns() const
{
	return { discerningEnumColumn, displayedEnumColumn };
}





/**
 * Creates a CompositeColumn.
 *
 * @param table		The CompositeTable that this column belongs to.
 * @param uiName	The name of this column as it should be displayed in the UI.
 * @param suffix	A suffix to append to the content of each cell.
 * @param sorting	The list of columns to sort by and their sort order, in order of priority.
 */
IndexCompositeColumn::IndexCompositeColumn(CompositeTable* table, QString name, QString uiName, QString suffix, const QList<QPair<Column* const, Qt::SortOrder>> sorting) :
		CompositeColumn(table, name, uiName, Qt::AlignRight, Integer, true, true, suffix),
		sorting(sorting)
{
	assert(!sorting.isEmpty());
	for (QPair<Column* const, Qt::SortOrder> sortPair : sorting) {
		assert(sortPair.first->table == table->getBaseTable());
		assert(sortPair.second == Qt::AscendingOrder || sortPair.second == Qt::DescendingOrder);
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
	int numberOfRows = sorting.at(0).first->table->getNumberOfRows();
	// Local order buffer which represents the ordered list of row indices
	QList<BufferRowIndex> order = QList<BufferRowIndex>();
	for (BufferRowIndex index = BufferRowIndex(0); index.isValid(numberOfRows); index++) {
		order.append(index);
	}
	
	for (int i = sorting.size() - 1; i >= 0; i--) {
		Column* const sortColumn	= sorting.at(i).first;
		Qt::SortOrder sortOrder		= sorting.at(i).second;
		
		auto comparator = [&sortColumn, sortOrder](BufferRowIndex i1, BufferRowIndex i2) {
			QVariant value1 = sortColumn->getValueAt(i1);
			QVariant value2 = sortColumn->getValueAt(i2);
			
			if (sortOrder == Qt::AscendingOrder) {
				return compareCells(sortColumn->type, value1, value2);
			} else {
				return compareCells(sortColumn->type, value2, value1);
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
const QSet<Column* const> IndexCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> columns = QSet<Column* const>();
	for (QPair<Column* const, Qt::SortOrder> sortPair : sorting) {
		columns += sortPair.first;
	}
	return columns;
}





/**
 * Creates a CompositeColumn.
 *
 * @param table		The CompositeTable that this column belongs to.
 * @param uiName	The name of this column as it should be displayed in the UI.
 * @param suffix	A suffix to append to the content of each cell.
 * @param sorting	The list of columns to sort by and their sort order, in order of priority. The first column automatically doubles as the separating (grouping) column.
 */
OrdinalCompositeColumn::OrdinalCompositeColumn(CompositeTable* table, QString name, QString uiName, QString suffix, const QList<QPair<Column* const, Qt::SortOrder>> sorting) :
	IndexCompositeColumn(table, name, uiName, suffix, sorting),
	separatingColumn(sorting.first().first)
{
	assert(sorting.first().first->isForeignKey());
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
		ItemID currentKey = separatingColumn->getValueAt(rowIndex);
		if (!currentKey.isValid()) {
			// No key, reset ordinal and append empty
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex.get(), QVariant());
		} else if (currentKey == lastKey) {
			// Same key, increase ordinal
			ordinal++;
			ordinals.replace(rowIndex.get(), ordinal);
		} else {
			// Next key, reset ordinal
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex.get(), ordinal);
		}
	}
	
	return ordinals;
}
