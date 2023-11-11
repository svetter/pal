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
 * @file composite_column.h
 * 
 * This file declares the CompositeColumn class and some of its subclasses.
 */

#ifndef COMPOSITE_COLUMN_H
#define COMPOSITE_COLUMN_H

#include "src/comp_tables/filter.h"
#include "src/db/column.h"
#include "src/settings/project_settings.h"

#include <QString>
#include <QVariant>

class CompositeTable;



/**
 * A column in a table which contains data which can be computed from multiple tables and columns
 * in one or multiple base tables.
 */
class CompositeColumn {
	/** The composite table this column belongs to. */
	CompositeTable* const table;
	
public:
	/** The internal name of this column. */
	const QString name;
	/** The name of this column for UI purposes. */
	const QString uiName;
	/** The alignment (left/center/right) of the content of this column. */
	const Qt::AlignmentFlag alignment;
	/** The data type of the content of this column. */
	const DataType contentType;
	/** Whether the cells of this column are interdependent (as opposed to each cell being computed separately). */
	const bool cellsAreInterdependent;
	
protected:
	/** The suffix to append to every cell value. */
	const QString suffix;
	/** A list of enum names to replace the raw cell content with. */
	const QStringList* const enumNames;
	/** A list of enum name lists to replace the raw cell content with. */
	const QList<QPair<QString, QStringList>>* const enumNameLists;
	
protected:
	CompositeColumn(CompositeTable* table, QString name, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, bool cellsAreInterdependent, QString suffix, const QStringList* enumNames = nullptr, const QList<QPair<QString, QStringList>>* enumNameLists = nullptr);
public:
	virtual ~CompositeColumn();
	
public:
	int getIndex() const;
	
	/**
	 * Computes the value of the cell at the given row index.
	 *
	 * @param rowIndex	The row index.
	 * @return			The computed value of the cell.
	 */
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const = 0;
	
	/**
	 * Computes the value of all cells in the column together.
	 *
	 * This is used for columns with interdependent cells, such as the IndexCompositeColumn.
	 *
	 * @return	Computed values for all cells in the column.
	 */
	virtual QList<QVariant> computeWholeColumn() const;
	
	QVariant getRawValueAt(BufferRowIndex rowIndex) const;
	QVariant getFormattedValueAt(BufferRowIndex rowIndex) const;
protected:
	QVariant replaceEnumIfApplicable(QVariant content) const;
public:
	QString toFormattedTableContent(QVariant rawCellContent) const;
	
	bool compare(const QVariant& value1, const QVariant& value2) const;
	
	void applySingleFilter(const Filter& filter, ViewOrderBuffer& orderBuffer) const;
	
	void announceChangedData() const;
	
	/**
	 * Returns a set of all columns in the base tables which are used to compute the content of
	 * this column.
	 * 
	 * @return	A set of all base table columns which are used to compute contents of this column.
	 */
	virtual const QSet<Column* const> getAllUnderlyingColumns() const = 0;
	
protected:
	ProjectSettings* getProjectSettings() const;
};



/**
 * A composite column which refers directly to a single column in a base table.
 */
class DirectCompositeColumn : public CompositeColumn {
	/** The column in the base table from which to take the content. */
	Column* const contentColumn;
	
public:
	DirectCompositeColumn(CompositeTable* table, QString name, QString uiName, Qt::AlignmentFlag alignment, QString suffix, Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



/**
 * A composite column which follows a series of foreign keys to a target base table and takes the
 * content from a column in that table.
 */
class ReferenceCompositeColumn : public CompositeColumn {
	/** The sequence of foreign key columns to follow to the target base table containing the content. */
	QList<Column*> foreignKeyColumnSequence;
	/** The column in the target base table from which to take the content. */
	Column* const contentColumn;
	
public:
	ReferenceCompositeColumn(CompositeTable* table, QString name, QString uiName, Qt::AlignmentFlag alignment, QString suffix, QList<Column*> foreignKeyColumnSequence, Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



/**
 * A composite column which computes a subtraction of two columns from the base table, both
 * containing either integers or dates.
 */
class DifferenceCompositeColumn : public CompositeColumn {
	/** The column containing the minuend. */
	Column* const minuendColumn;
	/** The column containing the subtrahend. */
	Column* const subtrahendColumn;
	
public:
	DifferenceCompositeColumn(CompositeTable* table, QString name, QString uiName, QString suffix, Column* minuendColumn, Column* subtrahendColumn);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



/**
 * A composite column which combines values from two enumerative columns from the base table and
 * uses them for a lookup in a two-dimensional string array.
 */
class DependentEnumCompositeColumn : public CompositeColumn {
	/** The first base table column, which determines the first dimension in the lookup. */
	Column* const discerningEnumColumn;
	/** The second base table column, which determines the second second dimension in the lookup. */
	Column* const displayedEnumColumn;
	
public:
	DependentEnumCompositeColumn(CompositeTable* table, QString name, QString uiName, Column* discerningEnumColumn, Column* displayedEnumColumn, const QList<QPair<QString, QStringList>>* enumNameLists);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



/** A composite column which indexes all rows in the table according to a given sorting. */
class IndexCompositeColumn : public CompositeColumn {
	/** The sorting to use for indexing, in order of last sorting round given first. */
	const QList<QPair<Column* const, Qt::SortOrder>> sorting;
	
public:
	IndexCompositeColumn(CompositeTable* table, QString name, QString uiName, QString suffix, const QList<QPair<Column* const, Qt::SortOrder>> sorting);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	QList<QVariant> computeWholeColumn() const override;
	QList<BufferRowIndex> getRowIndexOrderList() const;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};

/**
 * A composite column which sorts all rows in the base table, groups them by the content of a
 * given column, and indexes the rows within each group of equal contents in the separating column.
 */
class OrdinalCompositeColumn : public IndexCompositeColumn {
	/** The column to use for separating the rows into groups. */
	const Column* const separatingColumn;
	
public:
	OrdinalCompositeColumn(CompositeTable* table, QString name, QString uiName, QString suffix, const QList<QPair<Column* const, Qt::SortOrder>> sorting);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	QList<QVariant> computeWholeColumn() const override;
};



#endif // COMPOSITE_COLUMN_H
