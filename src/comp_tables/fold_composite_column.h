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

#ifndef FOLD_COMPOSITE_COLUMN_H
#define FOLD_COMPOSITE_COLUMN_H

#include "composite_column.h"



class FoldCompositeColumn : public CompositeColumn {
	const QList<QPair<Column*, Column*>> breadcrumbs;
protected:
	Column* const contentColumn;
	
public:
	FoldCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn = nullptr, const QStringList* enumNames = nullptr);
	
	QSet<int> evaluateBreadcrumbTrail(int rowIndex) const;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};

enum NumericFoldOp {
	CountFold,
	IDListFold,
	AverageFold,
	SumFold,
	MaxFold
};

class NumericFoldCompositeColumn : public FoldCompositeColumn {
	const NumericFoldOp op;
public:
	NumericFoldCompositeColumn(CompositeTable* table, QString uiName, NumericFoldOp op, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn = nullptr);
	
	virtual QVariant computeValueAt(int rowIndex) const override;
};

class ListStringFoldCompositeColumn : public FoldCompositeColumn {
public:
	ListStringFoldCompositeColumn(CompositeTable* table, QString uiName, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn);
	
	virtual QStringList formatAndSortIntoStringList(QSet<int>& rowIndexSet) const;
	virtual QVariant computeValueAt(int rowIndex) const override;
};

class HikerListCompositeColumn : public ListStringFoldCompositeColumn {
public:
	HikerListCompositeColumn(CompositeTable* table, QString uiName, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn);
	
	virtual QStringList formatAndSortIntoStringList(QSet<int>& rowIndexSet) const override;
	virtual QVariant computeValueAt(int rowIndex) const override;
};



#endif // FOLD_COMPOSITE_COLUMN_H
