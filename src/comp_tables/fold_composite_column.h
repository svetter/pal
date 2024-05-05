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
#include "numeric_fold_op.h"
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
public:
	/** The breadcrumbs, which are pairs of base table columns which lead to the content column. */
	const Breadcrumbs breadcrumbs;
	/** The column that contains the content to be folded. */
	const ValueColumn* const contentColumn;
	/** The normal table that contains the content to be folded. */
	const NormalTable* const contentTable;
	
protected:
	FoldCompositeColumn(CompColType type, CompositeTable& table, QString name, QString uiName, DataType contentType, bool isStatistical, QString suffix, const Breadcrumbs breadcrumbs, const ValueColumn* contentColumn = nullptr, const QStringList* enumNames = nullptr);
	
public:
	virtual QSet<ValidItemID> computeIDsAt(BufferRowIndex rowIndex) const override;
	
	virtual const QSet<const Column*> getAllUnderlyingColumns() const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override = 0;
};



/**
 * A FoldCompositeColumn that counts the number of associated entries.
 */
class CountFoldCompositeColumn : public FoldCompositeColumn {
	const NormalTable& countTable;
	
public:
	CountFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const NormalTable& countTable);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
public:
	static CountFoldCompositeColumn* decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db);
};



/**
 * A FoldCompositeColumn that folds numeric values, using one of the NumericFoldOp operations.
 * 
 * Entries can be counted, returned as a list of IDs, averaged, summed, or the maximum can be
 * determined.
 */
class NumericFoldCompositeColumn : public FoldCompositeColumn {
	/** The operation to perform when folding values. */
	const NumericFoldOp op;
	
public:
	NumericFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, NumericFoldOp op, const ValueColumn& contentColumn);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
public:
	static NumericFoldCompositeColumn* decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db);
};



/**
 * A composite column that folds values into a sorted comma separated list string.
 */
class ListStringFoldCompositeColumn : public FoldCompositeColumn {
public:
	ListStringFoldCompositeColumn(CompositeTable& table, QString name, QString uiName, const ValueColumn& contentColumn, const QStringList* enumNames = nullptr, bool isHikerList = false);
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	virtual QStringList formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
public:
	static ListStringFoldCompositeColumn* decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db);
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
	
	virtual QVariant computeValueAt(BufferRowIndex rowIndex) const override;
	virtual QStringList formatAndSortIntoStringList(QSet<BufferRowIndex>& rowIndexSet) const override;
};



#endif // FOLD_COMPOSITE_COLUMN_H
