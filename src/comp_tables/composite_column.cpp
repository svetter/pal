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

#include "src/db/normal_table.h"
#include "src/db/table.h"
#include "src/comp_tables/composite_table.h"
#include "src/data/enum_names.h"



CompositeColumn::CompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, QString suffix, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
		table(table),
		uiName(uiName),
		alignment(alignment),
		contentType(contentType),
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
	
	if (contentType == Date) {
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
	// return result of operation 'value1 < value2'
	bool value1Valid = value1.isValid();
	bool value2Valid = value2.isValid();
	
	if (!value1Valid && !value2Valid)	return false;
	
	switch (contentType) {
	case Integer:
		if (!value1Valid && value2Valid)	return true;
		if (value1Valid && !value2Valid)	return false;
		return value1.toInt() < value2.toInt();
	case Enum:
		assert(value1Valid && value2Valid);
		return value1.toInt() < value2.toInt();
	case DualEnum: {
		assert(value1Valid && value2Valid);
		assert(value1.canConvert<QList<QVariant>>() && value2.canConvert<QList<QVariant>>());
		QList<QVariant> intList1 = value1.toList();
		QList<QVariant> intList2 = value2.toList();
		assert(intList1.size() == 2 && intList2.size() == 2);
		assert(intList1.at(0).canConvert<int>() && intList2.at(0).canConvert<int>());
		int descerning1 = intList1.at(0).toInt();
		int descerning2 = intList2.at(0).toInt();
		if (descerning1 != descerning2) return descerning1 < descerning2;
		assert(intList1.at(1).canConvert<int>() && intList2.at(1).canConvert<int>());
		int displayed1 = intList1.at(1).toInt();
		int displayed2 = intList2.at(1).toInt();
		return displayed1 < displayed2;
	}
	case Bit:
		assert(value1Valid && value2Valid);
		assert(value1.canConvert<bool>() && value2.canConvert<bool>());
		return value1.toBool() < value2.toBool();
	case String:
		if (!value1Valid && value2Valid)	return false;
		if (value1Valid && !value2Valid)	return true;
		return QString::localeAwareCompare(value1.toString(), value2.toString()) < 0;
	case Date:
		if (!value1Valid && value2Valid)	return true;
		if (value1Valid && !value2Valid)	return false;
		assert(value1.canConvert<QDate>() && value2.canConvert<QDate>());
		return value1.toDate() < value2.toDate();
	case Time:
		if (!value1Valid && value2Valid)	return true;
		if (value1Valid && !value2Valid)	return false;
		assert(value1.canConvert<QTime>() && value2.canConvert<QTime>());
		return value1.toTime() < value2.toTime();
	default:
		assert(false);
		return false;
	}
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
		CompositeColumn(table, uiName, alignment, contentColumn->type, suffix, enumNames),
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
		CompositeColumn(table, uiName, alignment, contentColumn->type, suffix, enumNames),
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





FoldCompositeColumn::FoldCompositeColumn(CompositeTable* table, QString uiName, FoldOp op, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, op == ListString ? Qt::AlignLeft : Qt::AlignRight, op == ListString ? String : (op == Count ? Integer : op == IntList ? IDList : contentColumn->type), suffix, enumNames),
		op(op),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn)
{
	assert((op == Count) == (contentColumn == nullptr));
}



QVariant FoldCompositeColumn::computeValueAt(int rowIndex) const
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
			if (op == Count) return 0;
			return QVariant();
		}
	}
	
	// RETURN IF COUNT OR INT LIST
	if (op == Count) return currentRowIndexSet.size();
	if (op == IntList) {
		QList<QVariant> list = QList<QVariant>();
		for (int rowIndex : currentRowIndexSet) {
			list.append(contentColumn->getValueAt(rowIndex));
		}
		return list;
	}
	
	assert(currentTable == contentColumn->table);
	assert(!currentTable->isAssociative);
	
	// RETURN IF LIST STRING
	if (op == ListString) {
		QList<QString> hikerNameList;
		
		// Special case if this is a list of hikers
		if (contentColumn->name == contentColumn->name) {
			QString defaultHikerString = QString();
			const ProjectSetting<int>* defaultHiker = getProjectSettings()->defaultHiker;
			const HikersTable* hikersTable = (HikersTable*)contentColumn->table;
			int defaultHikerRowIndex = hikersTable->getBufferIndexForPrimaryKey(defaultHiker->get());
			if (defaultHiker->isNotNull() && currentRowIndexSet.contains(defaultHikerRowIndex)) {
				QVariant content = contentColumn->getValueAt(defaultHikerRowIndex);
				assert(content.canConvert<QString>());
				defaultHikerString = content.toString();
			}
			
			for (int rowIndex : currentRowIndexSet) {
				QVariant content = contentColumn->getValueAt(rowIndex);
				assert(content.canConvert<QString>());
				hikerNameList.append(content.toString());
			}
			std::sort(hikerNameList.begin(), hikerNameList.end());
			
			if (!defaultHikerString.isEmpty()) {
				hikerNameList.insert(0, defaultHikerString);
			}
		}
		else {
			for (int rowIndex : currentRowIndexSet) {
				QVariant content = contentColumn->getValueAt(rowIndex);
				assert(content.canConvert<QString>());
				content = replaceEnumIfApplicable(content);
				hikerNameList.append(content.toString());
			}
		}
		
		QString listString = "";
		for (QString& hikerNameString : hikerNameList) {
			if (!listString.isEmpty()) listString.append(", ");
			listString.append(hikerNameString);
		}
		return listString;
	}
	
	
	
	// EXECUTE FOLD OPERATION
	
	int aggregate = 0;
	
	for (int rowIndex : currentRowIndexSet) {
		QVariant content = contentColumn->getValueAt(rowIndex);
		
		switch (op) {
		case Average:
		case Sum:
			assert(content.canConvert<int>());
			aggregate += content.toInt();
			break;
		case Max:
			assert(content.canConvert<int>());
			if (content.toInt() > aggregate) aggregate = content.toInt();
			break;
		default:
			assert(false);
		}
	}
	
	assert(currentRowIndexSet.size() > 0);
	
	switch (op) {
	case Average:	return aggregate / currentRowIndexSet.size();
	case Sum:		return aggregate;
	case Max:		return aggregate;
	default:		assert(false);
	}
	
	return QVariant();
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





DifferenceCompositeColumn::DifferenceCompositeColumn(CompositeTable* table, QString uiName, QString suffix, Column* minuendColumn, Column* subtrahendColumn) :
		CompositeColumn(table, uiName, Qt::AlignRight, Integer, suffix),
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
		CompositeColumn(table, uiName, Qt::AlignLeft, DualEnum, QString(), nullptr, enumNameLists),
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
