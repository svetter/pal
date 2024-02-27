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
 * @file normal_table.h
 * 
 * This file declares the NormalTable class.
 */

#ifndef NORMAL_TABLE_H
#define NORMAL_TABLE_H

#include "table.h"
#include "src/data/item_id.h"

#include <QString>
#include <QList>



/**
 * A class for accessing and manipulating a normal (non-associative) table in the database.
 */
class NormalTable : public Table {
public:
	/** The primary key column of this table. */
	PrimaryKeyColumn primaryKeyColumn;
	
	NormalTable(QString name, QString uiName, const QString& primaryKeyColumnName, const QString& primaryKeyColumnUIName);
	virtual ~NormalTable();
	
	// Buffer access
	BufferRowIndex getBufferIndexForPrimaryKey(ValidItemID primaryKey) const;
	ValidItemID getPrimaryKeyAt(BufferRowIndex bufferRowIndex) const;
	QList<QPair<ValidItemID, QVariant>> pairIDWith(const Column& column) const;
	
	// Modifications (passthrough)
	BufferRowIndex addRow(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs);
	void updateCell(QWidget& parent, const ValidItemID primaryKey, const Column& column, const QVariant& data);
	void updateRow(QWidget& parent, const ValidItemID primaryKey, const QList<ColumnDataPair>& columnDataPairs);
	// removeRow(...) doesn't exist to avoid row removal without reference search. Outside interface is Database::removeRow(...)
	
	// Translation strings
	/**
	 * Returns the translated string to be displayed to indicate that no item is selected.
	 * 
	 * @return	The translated string representing absence of an item of the table's type.
	 */
	virtual QString getNoneString() const = 0;
	/**
	 * Returns the translated name of this table's item type in singular, not capitalized unless
	 * the language requires it.
	 * 
	 * @return	The translated name of the item type in singular for use mid-sentence.
	 */
	virtual QString getItemNameSingularLowercase() const = 0;
	/**
	 * Returns the translated name of this table's item type in plural, not capitalized unless the
	 * language requires it.
	 *
	 * @return	The translated name of the item type in plural for use mid-sentence.
	 */
	virtual QString getItemNamePluralLowercase() const = 0;
	
	// QAbstractItemModel implementation (completes implementation in Table)
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
};



#endif // NORMAL_TABLE_H
