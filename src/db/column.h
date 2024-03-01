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
 * @file column.h
 * 
 * This file declares the Column class.
 */

#ifndef COLUMN_H
#define COLUMN_H

#include "src/db/row_index.h"
#include "src/db/listeners.h"
#include "src/data/item_id.h"

#include <QSet>

using std::shared_ptr;

class PrimaryKeyColumn;
class Table;
struct WhatIfDeleteResult;



/**
 * Specifies the type of data contained in a database column.
 * 
 * Items correspond to SQL data types, except for the ID type.
 */
enum DataType {
	Integer, ID, Enum, DualEnum, Bit, String, Date, Time, IDList
};



/**
 * A general superclass for all column model classes for database tables.
 */
class Column {
public:
	/** The table this column belongs to. */
	const Table& table;
	
	/** The internal name of the column. */
	const QString name;
	/** The name of the column as it should be displayed in the UI. */
	const QString uiName;
	/** The type of data contained in the column. */
	const DataType type;
	/** Whether the column contains primary keys. */
	const bool primaryKey;
	/** The foreign column referenced by this column if it contains foreign keys. */
	PrimaryKeyColumn* const foreignColumn;
	/** Whether the column can contain null values. */
	const bool nullable;
	
	/** A list of enum names corresponding to the enum used for this column, or nullptr. */
	const QStringList* const enumNames;
	/** A list of enum name lists corresponding to the nested enum used for this column, or nullptr. */
	const QList<QPair<QString, QStringList>>* const enumNameLists;
	
private:
	/** The column change listeners registered to be notified when data in this column changes. */
	QSet<shared_ptr<const ColumnChangeListener>> changeListeners;
	
protected:
	Column(const Table& table, QString name, QString uiName, bool primaryKey, PrimaryKeyColumn* foreignKey, DataType type, bool nullable, const QStringList* enumNames = nullptr, const QList<QPair<QString, QStringList>>* enumNameLists = nullptr);
public:
	~Column();
	
	bool isPrimaryKey() const;
	bool isForeignKey() const;
	bool isKey() const;
	PrimaryKeyColumn& getReferencedForeignColumn() const;
	int getIndex() const;
	
	QVariant getValueAt(BufferRowIndex bufferRowIndex) const;
	QVariant getValueFor(ValidItemID itemID) const;
	bool anyCellMatches(QVariant value) const;
	
	QString getSqlSpecificationString() const;
	
	void registerChangeListener(shared_ptr<const ColumnChangeListener> newListener);
	const QSet<const ColumnChangeListener*> getChangeListeners() const;
};



/**
 * A class modelling a normal column holding some value in a database table.
 */
class ValueColumn : public Column
{
public:
	ValueColumn(const Table& table, QString name, QString uiName, DataType type, bool nullable, const QStringList* enumNames = nullptr, const QList<QPair<QString, QStringList>>* enumNameLists = nullptr);
};



/**
 * A class modelling a primary key column in a database table.
 */
class PrimaryKeyColumn : public Column
{
public:
	PrimaryKeyColumn(const Table& table, QString name, QString uiName);
};



/**
 * A class modelling a foreign key column in a database table.
 */
class ForeignKeyColumn : public Column
{
public:
	ForeignKeyColumn(const Table& table, QString name, QString uiName, bool nullable, PrimaryKeyColumn& foreignColumn, bool primaryKey = false);
};



/**
 * A class modelling a primary and foreign key column in a database table.
 */
class PrimaryForeignKeyColumn : public ForeignKeyColumn
{
public:
	PrimaryForeignKeyColumn(const Table& table, QString name, QString uiName, PrimaryKeyColumn& foreignColumn);
};



bool compareCells(DataType type, const QVariant& value1, const QVariant& value2);



QString getColumnListStringOf(QList<const Column*> columns);



QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults);



#endif // COLUMN_H
