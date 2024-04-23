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
	
	NormalTable(Database& db, QString name, QString uiName, const QString& primaryKeyColumnName, const QString& primaryKeyColumnUIName);
	virtual ~NormalTable();
	
	// Buffer access
	BufferRowIndex getBufferIndexForPrimaryKey(ValidItemID primaryKey) const;
	ValidItemID getPrimaryKeyAt(BufferRowIndex bufferRowIndex) const;
	QList<QPair<ValidItemID, QVariant>> pairIDWith(const Column& column) const;
	
	// Modifications (passthrough)
	BufferRowIndex addRow(QWidget& parent, const QList<ColumnDataPair>& columnDataPairs);
	void updateCell(QWidget& parent, const ValidItemID primaryKey, const Column& column, const QVariant& data);
	void updateRow(QWidget& parent, const ValidItemID primaryKey, QList<ColumnDataPair>& columnDataPairs);
	void updateRows(QWidget& parent, const QSet<BufferRowIndex>& bufferIndices, const QList<ColumnDataPair>& columnDataPairs);
	// removeRow(...) doesn't exist to avoid row removal without reference search. Outside interface is Database::removeRow(...)
	
	// Translation strings
	/**
	 * Returns the translation of the item name in singular form.
	 * 
	 * @return	The translation of the item name in singular form.
	 */
	virtual QString getItemNameSingular() const = 0;
	/**
	 * Returns the translated string to be displayed to indicate that no item is selected.
	 * 
	 * @return	The translated string representing absence of an item of the table's type.
	 */
	virtual QString getNoneString() const = 0;
	/**
	 * Returns a translated message confirming that a new item of this table's type has been
	 * created.
	 * 
	 * @return	The translated message confirming creation of a new item.
	 */
	virtual QString getCreationConfirmMessage() const = 0;
	/**
	 * Returns a translated message confirming that a number of items of this table's type have been
	 * edited.
	 * 
	 * @param numEdited	The number of items that have been edited.
	 * @return			The translated message confirming the editing of the items.
	 */
	virtual QString getEditConfirmMessage(int numEdited) const = 0;
	/**
	 * Returns a translated message confirming that a number of items of this table's type have been
	 * deleted.
	 * 
	 * @param numDeleted	The number of items that have been deleted.
	 * @return				The translated message confirming the deletion of the items.
	 */
	virtual QString getDeleteConfirmMessage(int numDeleted) const = 0;
	/**
	 * Returns a translated title for a wizard for creating a new custom column for the composite
	 * version of this table.
	 * 
	 * @return	The translated title for a wizard for creating a new custom column.
	 */
	virtual QString getNewCustomColumnString() const = 0;
	/**
	 * Returns a translated title for a filter wizard creating a new filter of this table's type.
	 * 
	 * @return	The translated title for a filter wizard creating a new filter.
	 */
	virtual QString getNewFilterString() const = 0;
	/**
	 * Returns a translated string which can be used in lists of items and their counts, in the form
	 * "n item(s)", e.g. "14 ascents" or "1 peak".
	 * 
	 * @param numItems	The number of items to list.
	 * @return			A translated list entry naming the item type and a corresponding count.
	 */
	virtual QString getItemCountString(int numItems) const = 0;
	
	// QAbstractItemModel implementation (completes implementation in Table)
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
};



#endif // NORMAL_TABLE_H
