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

#ifndef COLUMN_H
#define COLUMN_H

#include "src/data/item_id.h"
#include <QSet>

class Table;
struct WhatIfDeleteResult;
class CompositeColumn;



enum DataType {
	Integer, ID, Enum, DualEnum, Bit, String, Date, Time, IDList
};



class Column {
	QSet<const CompositeColumn*> changeListeners;
	
public:
	const QString		name;
	const QString		uiName;
	const DataType		type;
	const bool			primaryKey;
	Column* const		foreignKey;
	const bool			nullable;
	const Table* const	table;
	
	Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, Column* foreignKey, const Table* table);
	
	bool	isPrimaryKey() const;
	bool	isForeignKey() const;
	bool	isKey() const;
	Column*	getReferencedForeignColumn() const;
	int		getIndex() const;
	
	QVariant getValueAt(int bufferRowIndex) const;
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
