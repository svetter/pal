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
 * @file column.cpp
 * 
 * This file defines the Column class.
 */

#include "column.h"

#include "src/db/row_index.h"
#include "table.h"
#include "database.h"

#include <QCoreApplication>

using std::shared_ptr;



/**
 * Creates a new Column.
 * 
 * @param table			The table this column belongs to.
 * @param name			The internal name of the column.
 * @param uiName		The name of the column as it should be displayed in the UI.
 * @param primaryKey	Whether the column contains primary keys.
 * @param foreignColumn	The foreign column referenced by this column if it contains foreign keys.
 * @param type			The type of data contained in the column.
 * @param nullable		Whether the column may contain null values.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 * @param enumNameLists	An optional list of enum name lists with which to replace the raw cell content.
 */
Column::Column(const Table& table, QString name, QString uiName, bool primaryKey, PrimaryKeyColumn* foreignColumn, DataType type, bool nullable, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
	table(table),
	name(name),
	uiName(uiName),
	type(type),
	primaryKey(primaryKey),
	foreignColumn(foreignColumn),
	nullable(nullable),
	enumNames(enumNames),
	enumNameLists(enumNameLists),
	changeListeners(QSet<shared_ptr<const ColumnChangeListener>>())
{
	assert(name.compare(QString("ID"), Qt::CaseInsensitive) != 0);
	assert(table.isAssociative == (primaryKey && foreignColumn));
	if (primaryKey)						assert(!nullable);
	if (primaryKey || foreignColumn)	assert(type == ID && name.endsWith("ID"));
	if (name.endsWith("ID"))			assert((type == ID) && (primaryKey || foreignColumn));
}

/**
 * Destroys the Column.
 */
Column::~Column()
{}



/**
 * Indicates whether this column contains primary keys.
 * 
 * @return	True if this is a primary key column, false otherwise.
 */
bool Column::isPrimaryKey() const
{
	return primaryKey;
}

/**
 * Indicates whether this column contains foreign keys.
 * 
 * @return	True if this is a foreign key column, false otherwise.
 */
bool Column::isForeignKey() const
{
	return foreignColumn;
}

/**
 * Indicates whether this column contains keys (primary or foreign).
 * 
 * @return	True if this is a key column, false otherwise.
 */
bool Column::isKey() const
{
	return isPrimaryKey() || isForeignKey();
}

/**
 * Returns the column referenced by this column if it contains foreign keys, or nullptr otherwise.
 * 
 * @pre The column is a foreign key column.
 * 
 * @return	The foreign column referenced by this one, or nullptr.
 */
PrimaryKeyColumn& Column::getReferencedForeignColumn() const
{
	assert(foreignColumn);
	return *foreignColumn;
}

/**
 * Returns the index of this column in its table.
 * 
 * @return	The index of this column in its table.
 */
int Column::getIndex() const
{
	return table.getColumnIndex(*this);
}



/**
 * Returns the value stored in this column at the given row index.
 * 
 * @param rowIndex	The row index of the value to return.
 * @return			The value stored in this column at the given row index.
 */
QVariant Column::getValueAt(BufferRowIndex bufferRowIndex) const
{
	return table.getBufferRow(bufferRowIndex)->at(getIndex());
}

/**
 * Returns the value stored in this column in the row with the given primary key.
 * 
 * @pre This column is not in an associative table.
 * 
 * @param itemID	The primary key of the row to return the value from.
 * @return			The value in this column at the indicated row.
 */
QVariant Column::getValueFor(ValidItemID itemID) const
{
	assert(!table.isAssociative);
	return getValueAt(((NormalTable&) table).getBufferIndexForPrimaryKey(itemID));
}

/**
 * Checks whether the givevn value is found anywhere in this column.
 * 
 * @param value	The value to check for.
 * @return		True if this column contains the given value, false otherwise.
 */
bool Column::anyCellMatches(QVariant value) const
{
	for (BufferRowIndex index = BufferRowIndex(0); index.isValid(table.getNumberOfRows()); index++) {
		if (getValueAt(index) == value) return true;
	}
	return false;
}



/**
 * Assembles a string specifying the column's properties for a SQL CREATE TABLE statement.
 * 
 * @return	A string specifying the column's properties for a SQL query.
 */
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
	if (foreignColumn)
		primaryKeyString = " REFERENCES " + foreignColumn->table.name + "(" + foreignColumn->name + ")";
	
	QString nullString = "";
	if (!nullable)
		nullString = " NOT NULL";
	
	return name + " " + typeString + primaryKeyString + foreignKeyString + nullString;
}



/**
 * Registers the given column change listener for this column.
 * 
 * @param newListener	The change listener to register.
 */
void Column::registerChangeListener(shared_ptr<const ColumnChangeListener> newListener)
{
	changeListeners.insert(newListener);
}

/**
 * Returns the set of all change listeners registered for this column.
 */
const QSet<const ColumnChangeListener*> Column::getChangeListeners() const
{
	QSet<const ColumnChangeListener*> copySet = QSet<const ColumnChangeListener*>();
	for (const shared_ptr<const ColumnChangeListener>& listener : changeListeners) {
		copySet.insert(listener.get());
	}
	return copySet;
}





/**
 * Creates a ValueColumn.
 * 
 * @param table			The table this column belongs to.
 * @param name			The internal name of the column.
 * @param uiName		The name of the column as it should be displayed in the UI.
 * @param type			The type of data contained in the column.
 * @param nullable		Whether the column may contain null values.
 * @param enumNames		An optional list of enum names with which to replace the raw cell content.
 * @param enumNameLists	An optional list of enum name lists with which to replace the raw cell content.
 */
ValueColumn::ValueColumn(const Table& table, QString name, QString uiName, DataType type, bool nullable, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
	Column(table, name, uiName, false, nullptr, type, nullable, enumNames, enumNameLists)
{}



/**
 * Creates a PrimaryKeyColumn.
 * 
 * @param table		The table this column belongs to.
 * @param name		The internal name of the column.
 * @param uiName	The name of the column as it should be displayed in the UI.
 */
PrimaryKeyColumn::PrimaryKeyColumn(const Table& table, QString name, QString uiName) :
	Column(table, name, uiName, true, nullptr, ID, false)
{}



/**
 * Creates a ForeignKeyColumn.
 * 
 * @param table			The table this column belongs to.
 * @param name			The internal name of the column.
 * @param uiName		The name of the column as it should be displayed in the UI.
 * @param nullable		Whether the column may contain null values.
 * @param foreignColumn	The foreign column referenced by this column if it contains foreign keys.
 * @param primaryKey	Whether the column contains primary keys.
 */
ForeignKeyColumn::ForeignKeyColumn(const Table& table, QString name, QString uiName, bool nullable, PrimaryKeyColumn& foreignColumn, bool primaryKey) :
	Column(table, name, uiName, primaryKey, &foreignColumn, ID, nullable)
{}



/**
 * Creates a PrimaryForeignKeyColumn.
 * 
 * @param table		The table this column belongs to.
 * @param name		The internal name of the column.
 * @param uiName	The name of the column as it should be displayed in the UI.
 * @param foreignColumn	The foreign column referenced by this column if it contains foreign keys.
 */
PrimaryForeignKeyColumn::PrimaryForeignKeyColumn(const Table& table, QString name, QString uiName, PrimaryKeyColumn& foreignColumn) :
	ForeignKeyColumn(table, name, uiName, false, foreignColumn, true)
{}





/**
 * Compares two cells of the given type.
 * 
 * @param type		The type of the cells to compare.
 * @param value1	The first cell's value.
 * @param value2	The second cell's value.
 * @return			True if the first cell's value is less than the second cell's value, false otherwise.
 */
bool compareCells(DataType type, const QVariant& value1, const QVariant& value2)
{
	// return result of operation 'value1 < value2'
	bool value1Valid = value1.isValid();
	bool value2Valid = value2.isValid();
	
	if (!value1Valid && !value2Valid)	return false;
	if (!value1Valid &&  value2Valid)	return true;
	if ( value1Valid && !value2Valid)	return false;
	assert(value1Valid && value2Valid);
	
	switch (type) {
	case Integer:
	case ID:
		return value1.toInt() < value2.toInt();
	case Enum:
		return value1.toInt() < value2.toInt();
	case DualEnum: {
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
		assert(value1.canConvert<bool>() && value2.canConvert<bool>());
		return value1.toBool() < value2.toBool();
	case String:
		return QString::localeAwareCompare(value1.toString(), value2.toString()) < 0;
	case Date:
		assert(value1.canConvert<QDate>() && value2.canConvert<QDate>());
		return value1.toDate() < value2.toDate();
	case Time:
		assert(value1.canConvert<QTime>() && value2.canConvert<QTime>());
		return value1.toTime() < value2.toTime();
	default:
		assert(false);
		return false;
	}
}





/**
 * Returns a string listing the given columns' names.
 * 
 * @param columns	A list of columns.
 * @return			A comma-separated list of the columns' names.
 */
QString getColumnListStringOf(QList<const Column*> columns)
{
	QString result = "";
	bool first = true;
	for (const Column* column : columns) {
		result = result + (first ? "" : ", ") + column->name;
		first = false;
	}
	return result;
}



/**
 * Returns a translated string detailing the given singular consequence of deleting an item.
 * 
 * @param whatIfResult	The consequence of deleting an item as previously calculated.
 * @return				A translated string representing the consequence of deleting the item.
 */
QString getTranslatedWhatIfDeleteResultDescription(const WhatIfDeleteResult& whatIfResult)
{
	int numAffectedItems = whatIfResult.numAffectedRowIndices;
	QString itemName;
	if (numAffectedItems == 1) {
		itemName = whatIfResult.itemTable.getItemNameSingularLowercase();
	} else {
		itemName = whatIfResult.itemTable.getItemNamePluralLowercase();
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

/**
 * Returns a translated string listing the given consequences of deleting an item.
 * 
 * @param whatIfResults	The consequences of deleting an item as previously calculated.
 * @return				A translated string representing the consequences of deleting the item.
 */
QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults)
{
	QString baseString = QCoreApplication::translate("WhatIfDeleteResult", "The item will be removed from %1.");
	//: This goes in between any two entries in a list, but not before the last entry
	QString listSeparatorString = QCoreApplication::translate("WhatIfDeleteResult", ", ");
	//: This goes in between the last and second-to-last entries in a list
	QString oxfordCommaString = QCoreApplication::translate("WhatIfDeleteResult", ", and ");
	
	QString argumentString = "";
	int iterationsLeft = whatIfResults.size();
	for (const WhatIfDeleteResult whatIfResult : whatIfResults) {
		argumentString.append(getTranslatedWhatIfDeleteResultDescription(whatIfResult));
		
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
