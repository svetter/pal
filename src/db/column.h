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
 * @file column.h
 * 
 * This file declares the Column class.
 */

#ifndef COLUMN_H
#define COLUMN_H

#include "src/db/row_index.h"
#include "src/data/item_id.h"
#include <QSet>

class Table;
struct WhatIfDeleteResult;
class CompositeColumn;



/**
 * Specifies the type of data contained in a database column.
 * 
 * Items correspond to SQL data types, except for the ID type.
 */
enum DataType {
	Integer, ID, Enum, DualEnum, Bit, String, Date, Time, IDList
};


/**
 * A class modelling a column in a database table.
 */
class Column {
	/** The composite columnd which have registered to be notified when data in this column changes. */
	QSet<const CompositeColumn*> changeListeners;
	
public:
	/** The internal name of the column. */
	const QString		name;
	/** The name of the column as it should be displayed in the UI. */
	const QString		uiName;
	/** The type of data contained in the column. */
	const DataType		type;
	/** Whether the column contains primary keys. */
	const bool			primaryKey;
	/** The foreign column referenced by this column if it contains foreign keys. */
	Column* const		foreignKey;
	/** Whether the column can contain null values. */
	const bool			nullable;
	/** The table this column belongs to. */
	const Table* const	table;
	
	/** A list of enum names corresponding to the enum used for this column, or nullptr. */
	const QStringList* const enumNames;
	/** A list of enum name lists corresponding to the nested enum used for this column, or nullptr. */
	const QList<QPair<QString, QStringList>>* const enumNameLists;
	
	Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, Column* foreignKey, const Table* table, const QStringList* enumNames = nullptr, const QList<QPair<QString, QStringList>>* enumNameLists = nullptr);
	
	bool	isPrimaryKey() const;
	bool	isForeignKey() const;
	bool	isKey() const;
	Column*	getReferencedForeignColumn() const;
	int		getIndex() const;
	
	QVariant getValueAt(BufferRowIndex bufferRowIndex) const;
	QVariant getValueFor(ValidItemID itemID) const;
	bool anyCellMatches(QVariant value) const;
	
	QString getSqlSpecificationString() const;
	
	void registerChangeListener(const CompositeColumn* compositeColumn);
	QSet<const CompositeColumn*> getChangeListeners() const;
};



bool compareCells(DataType type, const QVariant& value1, const QVariant& value2);



QString getColumnListStringOf(QList<const Column*> columns);



QString getTranslatedWhatIfDeleteResultDescription(const WhatIfDeleteResult& whatIfResult);
QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults);



#endif // COLUMN_H
