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
 * @file composite_column.h
 * 
 * This file declares the CompositeColumn class and some of its subclasses.
 */

#ifndef COMPOSITE_COLUMN_H
#define COMPOSITE_COLUMN_H

#include "src/settings/string_encoder.h"
#include "comp_column_type.h"
#include "src/db/breadcrumbs.h"
#include "src/db/column.h"
#include "src/settings/project_settings.h"

#include <QString>
#include <QVariant>

class CompositeTable;
class ItemTypesHandler;



/**
 * A column in a table which contains data which can be computed from multiple tables and columns in
 * one or multiple base tables.
 */
class CompositeColumn : public StringEncoder {
public:
	const CompColType type;
	
	/** The composite table this column belongs to. */
	CompositeTable& table;
	
	/** The internal name of this column. */
	const QString name;
	/** The name of this column for UI purposes. */
	const QString uiName;
	/** The data type of the content of this column. */
	const DataType contentType;
	/** The alignment (left/center/right) of the content of this column. */
	const Qt::AlignmentFlag alignment;
	/** Whether the cells of this column are interdependent (as opposed to each cell being computed separately). */
	const bool cellsAreInterdependent;
	/** Whether this column is a statistical column, which only holds information derived from other columns. */
	const bool isStatistical;
	
	/** A list of enum names to replace the raw cell content with. */
	const QStringList* const enumNames;
	/** A list of enum name lists to replace the raw cell content with. */
	const QList<QPair<QString, QStringList>>* const enumNameLists;
	
protected:
	/** The suffix to append to every cell value. */
	const QString suffix;
	
protected:
	CompositeColumn(CompColType type, CompositeTable& table, QString name, QString uiName, DataType contentType, bool cellsAreInterdependent, bool isStatistical, QString suffix, const QStringList* enumNames = nullptr, const QList<QPair<QString, QStringList>>* enumNameLists = nullptr);
public:
	virtual ~CompositeColumn();
	
public:
	int getIndex() const;
	int getExportIndex() const;
	bool isExportOnlyColumn() const;
	bool isFilterOnlyColumn() const;
	
	virtual QSet<ValidItemID> computeIDsAt(BufferRowIndex rowIndex) const;
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
	
	/**
	 * Returns a set of all columns in the base tables which are used to compute the content of
	 * this column.
	 * 
	 * @return	A set of all base table columns which are used to compute contents of this column.
	 */
	virtual const QSet<const Column*> getAllUnderlyingColumns() const = 0;
	
protected:
	const ProjectSettings& getProjectSettings() const;
	
	
	
	// Encoding & Decoding
public:
	static QString encodeToString(QList<const CompositeColumn*> columns);
private:
	QString encodeSingleColumnToString() const;
public:
	static QList<CompositeColumn*> decodeFromString(const QString& encoded, Database& db, const ItemTypesHandler& typesHandler);
private:
	static CompositeColumn* decodeSingleColumnFromString(QString& restOfString, Database& db, const ItemTypesHandler& typesHandler);
protected:
	virtual QStringList encodeTypeSpecific() const = 0;
};



/**
 * A composite column which refers directly to a single column in a base table.
 */
class DirectCompositeColumn : public CompositeColumn {
public:
	/** The column in the base table from which to take the content. */
	const Column& contentColumn;
	
	DirectCompositeColumn(CompositeTable& table, QString suffix, const Column& contentColumn);
	DirectCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const Column& contentColumn);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<const Column*> getAllUnderlyingColumns() const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
public:
	static DirectCompositeColumn* decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db);
};



/**
 * A composite column which follows a series of foreign keys to a target base table and takes the
 * content from a column in that table.
 */
class ReferenceCompositeColumn : public CompositeColumn {
public:
	/** The breadcrumb trail (chain of foreign key columns) to follow to the target table containing the content. */
	const Breadcrumbs breadcrumbs;
	/** The column in the target base table from which to take the content. */
	const Column& contentColumn;
	
	ReferenceCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const Column& contentColumn);
	
	virtual QSet<ValidItemID> computeIDsAt(BufferRowIndex rowIndex) const override;
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<const Column*> getAllUnderlyingColumns() const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
public:
	static ReferenceCompositeColumn* decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db);
};



/**
 * A composite column which computes a subtraction of two columns from the base table, both
 * containing either integers or dates.
 */
class DifferenceCompositeColumn : public CompositeColumn {
	/** The column containing the minuend. */
	ValueColumn& minuendColumn;
	/** The column containing the subtrahend. */
	ValueColumn& subtrahendColumn;
	
public:
	DifferenceCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, ValueColumn& minuendColumn, ValueColumn& subtrahendColumn);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<const Column*> getAllUnderlyingColumns() const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
};



/**
 * A composite column which combines values from two enumerative columns from the base table and
 * uses them for a lookup in a two-dimensional string array.
 */
class DependentEnumCompositeColumn : public CompositeColumn {
	/** The first base table column, which determines the first dimension in the lookup. */
	ValueColumn& discerningEnumColumn;
	/** The second base table column, which determines the second second dimension in the lookup. */
	ValueColumn& displayedEnumColumn;
	
public:
	DependentEnumCompositeColumn(CompositeTable& table, QString name, QString uiName, ValueColumn& discerningEnumColumn, ValueColumn& displayedEnumColumn);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<const Column*> getAllUnderlyingColumns() const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
};



/**
 * A struct representing a single pass of sorting base tables according to a column and sort order.
 */
struct BaseSortingPass {
	Column& column;
	const Qt::SortOrder order;
};



/**
 * A composite column which indexes all rows in the table according to a given sorting.
 */
class IndexCompositeColumn : public CompositeColumn {
	/** The sorting to use for indexing, in order of last sorting round given first. */
	const QList<BaseSortingPass> sortingPasses;
	
public:
	IndexCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const QList<BaseSortingPass> sortingPasses, bool isOrdinal = false);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	QList<QVariant> computeWholeColumn() const override;
	QList<BufferRowIndex> getRowIndexOrderList() const;
	
	virtual const QSet<const Column*> getAllUnderlyingColumns() const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
};

/**
 * A composite column which sorts all rows in the base table, groups them by the content of a
 * given column, and indexes the rows within each group of equal contents in the separating column.
 */
class OrdinalCompositeColumn : public IndexCompositeColumn {
	/** The column to use for separating the rows into groups. */
	const Column& separatingColumn;
	
public:
	OrdinalCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const QList<BaseSortingPass> sortingPasses);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	QList<QVariant> computeWholeColumn() const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
};



#endif // COMPOSITE_COLUMN_H
