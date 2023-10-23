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

#include "column.h"

#include "table.h"
#include "database.h"
#include "src/comp_tables/composite_column.h"

#include <QCoreApplication>



Column::Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, Column* foreignKey, const Table* table) :
		changeListeners(QSet<const CompositeColumn*>()),
		name(name),
		uiName(uiName),
		type(type),
		primaryKey(primaryKey),
		foreignKey(foreignKey),
		nullable(nullable),
		table(table)
{
	assert(name.compare(QString("ID"), Qt::CaseInsensitive) != 0);
	assert(table->isAssociative == (primaryKey && foreignKey));
	if (primaryKey)					assert(!nullable);
	if (primaryKey || foreignKey)	assert(type == ID && name.endsWith("ID"));
	if (name.endsWith("ID"))		assert((type == ID) && (primaryKey || foreignKey));
}



bool Column::isPrimaryKey() const
{
	return primaryKey;
}

bool Column::isForeignKey() const
{
	return foreignKey;
}

bool Column::isKey() const
{
	return isPrimaryKey() || isForeignKey();
}

Column* Column::getReferencedForeignColumn() const
{
	return foreignKey;
}

int Column::getIndex() const
{
	return table->getColumnIndex(this);
}



QVariant Column::getValueAt(int bufferRowIndex) const
{
	return table->getBufferRow(bufferRowIndex)->at(getIndex());
}

QVariant Column::getValueFor(ValidItemID itemID) const
{
	assert(!table->isAssociative);
	return getValueAt(((NormalTable*) table)->getBufferIndexForPrimaryKey(itemID));
}

bool Column::anyCellMatches(QVariant value) const
{
	for (int i = 0; i < table->getNumberOfRows(); i++) {
		if (getValueAt(i) == value) return true;
	}
	return false;
}



QString Column::getSqlSpecificationString() const
{
	QString typeString;
	switch (type) {
	case Integer:
	case ID:
	case Enum:
	case DualEnum:	typeString = "INTEGER";		break;
	case Bit:		typeString = "BIT";			break;
	case String:	typeString = "NVARCHAR";	break;
	case Date:		typeString = "DATE";		break;
	case Time:		typeString = "TIME";		break;
	default: assert(false);
	}
	
	QString primaryKeyString = "";
	if (primaryKey)
		primaryKeyString = " PRIMARY KEY";
	
	QString foreignKeyString = "";
	if (foreignKey)
		primaryKeyString = " REFERENCES " + foreignKey->table->name + "(" + foreignKey->name + ")";
	
	QString nullString = "";
	if (!nullable)
		nullString = " NOT NULL";
	
	return name + " " + typeString + primaryKeyString + foreignKeyString + nullString;
}



void Column::registerChangeListener(const CompositeColumn* compositeColumn)
{
	changeListeners.insert(compositeColumn);
}

QSet<const CompositeColumn*> Column::getChangeListeners() const
{
	return changeListeners;
}





bool compareCells(DataType type, const QVariant& value1, const QVariant& value2)
{
	// return result of operation 'value1 < value2'
	bool value1Valid = value1.isValid();
	bool value2Valid = value2.isValid();
	
	if (!value1Valid && !value2Valid)	return false;
	
	switch (type) {
	case Integer:
	case ID:
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





QString getColumnListStringOf(QList<const Column*> columns)
{
	QString result = "";
	bool first = true;
	for (auto iter = columns.begin(); iter != columns.end(); iter++) {
		result = result + (first ? "" : ", ") + (*iter)->name;
		first = false;
	}
	return result;
}



QString getTranslatedWhatIfDeleteResultDescription(const WhatIfDeleteResult& whatIfResult)
{
	int numAffectedItems = whatIfResult.numAffectedRowIndices;
	QString itemName;
	if (numAffectedItems == 1) {
		itemName = whatIfResult.itemTable->getItemNameSingularLowercase();
	} else {
		itemName = whatIfResult.itemTable->getItemNamePluralLowercase();
	}
	/*: This will be part of a listing of consequences of deleting an item.
	 *  An example would be: "Hiker will be removed from 42 ascents."
	 *  Here, the string "42 ascents" would have been the result of using the base string "%1 %2"
	 *  and replacing "%1" with "42" and "%2% with "ascents".
	 *  A translation for %2 is retrieved from elsewhere.
	 */
	QString baseString = QCoreApplication::translate("WhatIfDeleteResult", "%1 %2");
	return baseString.arg(numAffectedItems).arg(itemName);
}

QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults)
{
	QString baseString = QCoreApplication::translate("WhatIfDeleteResult", "The item will be removed from %1.");
	//: This goes in between any two entries in a list, but not before the last entry
	QString listSeparatorString = QCoreApplication::translate("WhatIfDeleteResult", ", ");
	//: This goes in between the last and second-to-last entries in a list
	QString oxfordCommaString = QCoreApplication::translate("WhatIfDeleteResult", ", and ");
	
	QString argumentString = "";
	int iterationsLeft = whatIfResults.size();
	for (auto iter = whatIfResults.constBegin(); iter != whatIfResults.constEnd(); iter++) {
		argumentString.append(getTranslatedWhatIfDeleteResultDescription(*iter));
		
		switch (--iterationsLeft) {
		case 0:
			break;
		case 1:
			argumentString.append(oxfordCommaString);
			break;
		default:
			assert(iterationsLeft > 0);
			argumentString.append(listSeparatorString);
		}
	}
	
	return baseString.arg(argumentString);
}
