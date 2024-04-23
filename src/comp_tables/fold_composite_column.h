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
 * @file fold_composite_column.h
 * 
 * This file declares the FoldCompositeColumn class and its subclasses.
 */

#ifndef FOLD_COMPOSITE_COLUMN_H
#define FOLD_COMPOSITE_COLUMN_H

#include "composite_column.h"
#include "fold_op.h"
#include "src/db/breadcrumbs.h"



/**
 * A composite column that follows a trail of "breadcrumbs" to a content column, collecting a set of
 * buffer row indices, and folds these rows into a single value.
 * 
 * The breadcrumbs are a list of pairs of columns which lead from the CompositeTable's underlying
 * database table to the content column, following a path through the database topology specified by
 * the breadcrumbs. This process leads to a set of buffer row indices, which are then folded into a
 * single value.
 */
class FoldCompositeColumn : public CompositeColumn {
protected:
	/** The operation to perform when folding values. */
	const FoldOp op;
	/** The breadcrumbs, which are pairs of base table columns which lead to the content column. */
	const Breadcrumbs breadcrumbs;
	/** The column that contains the content to be folded. */
	const Column* const contentColumn;
	
	FoldCompositeColumn(CompositeTable& table, QString name, QString uiName, DataType contentType, bool isStatistical, QString suffix, FoldOp op, const Breadcrumbs breadcrumbs, const Column* contentColumn = nullptr, const QStringList* enumNames = nullptr);
	
public:
	virtual const QSet<const Column*> getAllUnderlyingColumns() const override;
};



/**
 * A FoldCompositeColumn that folds numeric values, using one of the NumericFoldOp operations.
 * 
 * Entries can be counted, returned as a list of IDs, averaged, summed, or the maximum can be
 * determined.
 */
class NumericFoldCompositeColumn : public FoldCompositeColumn {
public:
	NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, FoldOp op, DataType contentType, const Breadcrumbs breadcrumbs, const Column* contentColumn);
	NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, FoldOp op, const Breadcrumbs breadcrumbs);
	NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, FoldOp op, const Column& contentColumn);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
};



/**
 * A composite column that folds values into a sorted comma separated list string.
 */
class ListStringFoldCompositeColumn : public FoldCompositeColumn {
public:
	ListStringFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, const Column& contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QStringList formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const;
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
};



/**
 * A specialized ListStringFoldCompositeColumn that folds hiker names into a sorted comma separated
 * list string.
 * 
 * This class exists to enable always having the default hiker at the first position of a hiker
 * list.
 */
class HikerListFoldCompositeColumn : public ListStringFoldCompositeColumn {
public:
	HikerListFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, const ValueColumn& contentColumn);
	
	virtual QStringList formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const override;
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
};



#endif // FOLD_COMPOSITE_COLUMN_H
