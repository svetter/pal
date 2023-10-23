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

#include "composite_column.h"

#include "src/comp_tables/composite_table.h"
#include "src/data/enum_names.h"



CompositeColumn::CompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, bool cellsAreInterdependent, QString suffix, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
		table(table),
		uiName(uiName),
		alignment(alignment),
		contentType(contentType),
		cellsAreInterdependent(cellsAreInterdependent),
		suffix(suffix),
		enumNames(enumNames),
		enumNameLists(enumNameLists)
{}

CompositeColumn::~CompositeColumn()
{}



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



int CompositeColumn::getIndex() const
{
	return table->getIndexOf(this);
}



QList<QVariant> CompositeColumn::computeWholeColumn() const
{
	QList<QVariant> cells = QList<QVariant>();
	for (int rowIndex = 0; rowIndex < table->getBaseTable()->getNumberOfRows(); rowIndex++) {
		cells.append(computeValueAt(rowIndex));
	}
	return cells;
}



QVariant CompositeColumn::getRawValueAt(int rowIndex) const
{
	return table->getRawValue(rowIndex, this);
}

QVariant CompositeColumn::getFormattedValueAt(int rowIndex) const
{
	return table->getFormattedValue(rowIndex, this);
}



QVariant CompositeColumn::replaceEnumIfApplicable(QVariant content) const
{
	if (!enumNames) return content;
	
	// If content is enumerative, replace index with the corresponding string
	assert(content.canConvert<int>());
	int index = content.toInt();
	assert(index >= 0 && index < enumNames->size());
	return EnumNames::tr(enumNames->at(index).toStdString().c_str());
}



bool CompositeColumn::compare(const QVariant& value1, const QVariant& value2) const
{
	return compareCells(contentType, value1, value2);
}



void CompositeColumn::applySingleFilter(const Filter& filter, QList<int>& orderBuffer) const
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
	for (int i = orderBuffer.size() - 1; i >= 0; i--) {
		bool letThrough = valuePasses(getRawValueAt(orderBuffer.at(i)));
		if (letThrough) continue;
		orderBuffer.removeAt(i);
	}
}



void CompositeColumn::announceChangedData() const
{
	int thisColumnIndex = table->getIndexOf(this);
	table->announceChangesUnderColumn(thisColumnIndex);
}



ProjectSettings* CompositeColumn::getProjectSettings() const
{
	return table->getProjectSettings();
}





DirectCompositeColumn::DirectCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, QString suffix, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, contentColumn->type, false, suffix, enumNames),
		contentColumn(contentColumn)
{
	assert(contentColumn);
}



QVariant DirectCompositeColumn::computeValueAt(int rowIndex) const
{
	return contentColumn->getValueAt(rowIndex);
}



const QSet<Column* const> DirectCompositeColumn::getAllUnderlyingColumns() const
{
	return { contentColumn };
}





ReferenceCompositeColumn::ReferenceCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, QString suffix, QList<Column*> foreignKeyColumnSequence, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, contentColumn->type, false, suffix, enumNames),
		foreignKeyColumnSequence(foreignKeyColumnSequence),
		contentColumn(contentColumn)
{
	assert(contentColumn);
}



QVariant ReferenceCompositeColumn::computeValueAt(int rowIndex) const
{
	assert(foreignKeyColumnSequence.first()->isForeignKey());
	
	int currentRowIndex = rowIndex;
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
		currentRowIndex = currentTable->getBufferIndexForPrimaryKey(key.forceValid());
		assert(currentRowIndex >= 0);
	}
	
	// Finally, look up content column at last row index
	assert(contentColumn->table == currentTable);
	QVariant content = contentColumn->getValueAt(currentRowIndex);
	
	return content;
}



const QSet<Column* const> ReferenceCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> result = { contentColumn };
	for (Column* column : foreignKeyColumnSequence) {
		result.insert(column);
		result.insert(column->getReferencedForeignColumn());
	}
	return result;
}





DifferenceCompositeColumn::DifferenceCompositeColumn(CompositeTable* table, QString uiName, QString suffix, Column* minuendColumn, Column* subtrahendColumn) :
		CompositeColumn(table, uiName, Qt::AlignRight, Integer, false, suffix),
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



QVariant DifferenceCompositeColumn::computeValueAt(int rowIndex) const
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



const QSet<Column* const> DifferenceCompositeColumn::getAllUnderlyingColumns() const
{
	return { minuendColumn, subtrahendColumn };
}





DependentEnumCompositeColumn::DependentEnumCompositeColumn(CompositeTable* table, QString uiName, Column* discerningEnumColumn, Column* displayedEnumColumn, const QList<QPair<QString, QStringList>>* enumNameLists) :
		CompositeColumn(table, uiName, Qt::AlignLeft, DualEnum, false, QString(), nullptr, enumNameLists),
		discerningEnumColumn(discerningEnumColumn),
		displayedEnumColumn(displayedEnumColumn)
{
	assert(discerningEnumColumn && displayedEnumColumn);
	assert(discerningEnumColumn->table == displayedEnumColumn->table);
	assert(!discerningEnumColumn->isKey() && !displayedEnumColumn->isKey());
	assert(discerningEnumColumn->type == DualEnum && displayedEnumColumn->type == DualEnum);
	assert(discerningEnumColumn != displayedEnumColumn);
}



QVariant DependentEnumCompositeColumn::computeValueAt(int rowIndex) const
{
	QVariant discerningContent = discerningEnumColumn->getValueAt(rowIndex);
	QVariant displayedContent = displayedEnumColumn->getValueAt(rowIndex);
	
	assert(discerningContent.canConvert<int>() && displayedContent.canConvert<int>());
	int discerning = discerningContent.toInt();
	int displayed = displayedContent.toInt();
	
	if (discerning < 1 || displayed < 1) return QVariant();
	
	return QVariant(QList<QVariant>({ discerning, displayed }));
}



const QSet<Column* const> DependentEnumCompositeColumn::getAllUnderlyingColumns() const
{
	return { discerningEnumColumn, displayedEnumColumn };
}





IndexCompositeColumn::IndexCompositeColumn(CompositeTable* table, QString uiName, QString suffix, const QList<QPair<Column* const, Qt::SortOrder>> sorting) :
		CompositeColumn(table, uiName, Qt::AlignRight, Integer, true, suffix),
		sorting(sorting)
{
	assert(!sorting.isEmpty());
	for (QPair<Column* const, Qt::SortOrder> sortPair : sorting) {
		assert(sortPair.first->table == table->getBaseTable());
		assert(sortPair.second == Qt::AscendingOrder || sortPair.second == Qt::DescendingOrder);
	}
}



QVariant IndexCompositeColumn::computeValueAt(int rowIndex) const
{
	QList<int> order = getRowIndexOrderList();
	return order.indexOf(rowIndex) + 1;
}

QList<QVariant> IndexCompositeColumn::computeWholeColumn() const
{
	QList<int> order = getRowIndexOrderList();
	
	QList<QVariant> cells = QList<QVariant>();
	for (int rowIndex = 0; rowIndex < order.size(); rowIndex++) {
		cells.append(order.indexOf(rowIndex) + 1);
	}
	
	return cells;
}

QList<int> IndexCompositeColumn::getRowIndexOrderList() const
{
	int numberOfRows = sorting.at(0).first->table->getNumberOfRows();
	// Local order buffer which represents the ordered list of row indices
	QList<int> order = QList<int>();
	for (int i = 0; i < numberOfRows; i++) {
		order += i;
	}
	
	for (int i = sorting.size() - 1; i >= 0; i--) {
		Column* const sortColumn	= sorting.at(i).first;
		Qt::SortOrder sortOrder		= sorting.at(i).second;
		
		auto comparator = [&sortColumn, sortOrder](int i1, int i2) {
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



const QSet<Column* const> IndexCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> columns = QSet<Column* const>();
	for (QPair<Column* const, Qt::SortOrder> sortPair : sorting) {
		columns += sortPair.first;
	}
	return columns;
}





OrdinalCompositeColumn::OrdinalCompositeColumn(CompositeTable* table, QString uiName, QString suffix, const QList<QPair<Column* const, Qt::SortOrder>> sorting) :
	IndexCompositeColumn(table, uiName, suffix, sorting),
	separatingColumn(sorting.first().first)
{
	assert(sorting.first().first->isForeignKey());
}



QVariant OrdinalCompositeColumn::computeValueAt(int rowIndex) const
{
	qDebug() << "CAUTION: Using extremely inefficient OrdinalCompositeColumn::computeValueAt(int)";
	return computeWholeColumn().at(rowIndex);
}

QList<QVariant> OrdinalCompositeColumn::computeWholeColumn() const
{
	QList<int> order = getRowIndexOrderList();
	
	QList<QVariant> ordinals = QList<QVariant>(order.size());
	
	ItemID lastKey = ItemID();
	int ordinal = 1;
	for (int rowIndex : order) {
		ItemID currentKey = separatingColumn->getValueAt(rowIndex);
		if (!currentKey.isValid()) {
			// No key, reset ordinal and append empty
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex, QVariant());
		} else if (currentKey == lastKey) {
			// Same key, increase ordinal
			ordinal++;
			ordinals.replace(rowIndex, ordinal);
		} else {
			// Next key, reset ordinal
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex, ordinal);
		}
	}
	
	return ordinals;
}
