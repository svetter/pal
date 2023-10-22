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

#ifndef COMPOSITE_COLUMN_H
#define COMPOSITE_COLUMN_H

#include "src/comp_tables/filter.h"
#include "src/db/column.h"
#include "src/db/project_settings.h"

#include <QString>
#include <QVariant>

class CompositeTable;



class CompositeColumn {
	CompositeTable* const table;
	
public:
	const QString uiName;
	const Qt::AlignmentFlag alignment;
	const DataType contentType;
	const bool cellsAreInterdependent;
	
protected:
	const QString suffix;
	const QStringList* const enumNames;
	const QList<QPair<QString, QStringList>>* const enumNameLists;
	
protected:
	CompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, bool cellsAreInterdependent, QString suffix, const QStringList* enumNames = nullptr, const QList<QPair<QString, QStringList>>* enumNameLists = nullptr);
public:
	virtual ~CompositeColumn();
	
public:
	int getIndex() const;
	
	virtual QVariant computeValueAt(int rowIndex) const = 0;
	virtual QList<QVariant> computeWholeColumn() const;
	
	QVariant getRawValueAt(int rowIndex) const;
	QVariant getFormattedValueAt(int rowIndex) const;
protected:
	QVariant replaceEnumIfApplicable(QVariant content) const;
public:
	QString toFormattedTableContent(QVariant rawCellContent) const;
	
	bool compare(const QVariant& value1, const QVariant& value2) const;
	
	void applySingleFilter(const Filter& filter, QList<int>& orderBuffer) const;
	
	void announceChangedData() const;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const = 0;
	
protected:
	ProjectSettings* getProjectSettings() const;
};



class DirectCompositeColumn : public CompositeColumn {
	Column* const contentColumn;
	
public:
	DirectCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, QString suffix, Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant computeValueAt(int rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



class ReferenceCompositeColumn : public CompositeColumn {
	QList<Column*> foreignKeyColumnSequence;
	Column* const contentColumn;
	
public:
	ReferenceCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, QString suffix, QList<Column*> foreignKeyColumnSequence, Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant computeValueAt(int rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



enum FoldOp {
	Count,
	IntList,
	ListString,
	Average,
	Sum,
	Max
};

class FoldCompositeColumn : public CompositeColumn {
	const FoldOp op;
	const QList<QPair<Column*, Column*>> breadcrumbs;
	Column* const contentColumn;
	
public:
	FoldCompositeColumn(CompositeTable* table, QString uiName, FoldOp op, QString suffix, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn = nullptr, const QStringList* enumNames = nullptr);
	
	virtual QVariant computeValueAt(int rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



class DifferenceCompositeColumn : public CompositeColumn {
	Column* const minuendColumn;
	Column* const subtrahendColumn;
	
public:
	DifferenceCompositeColumn(CompositeTable* table, QString uiName, QString suffix, Column* minuendColumn, Column* subtrahendColumn);
	
	virtual QVariant computeValueAt(int rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



class DependentEnumCompositeColumn : public CompositeColumn {
	Column* const discerningEnumColumn;
	Column* const displayedEnumColumn;
	
public:
	DependentEnumCompositeColumn(CompositeTable* table, QString uiName, Column* discerningEnumColumn, Column* displayedEnumColumn, const QList<QPair<QString, QStringList>>* enumNameLists);
	
	virtual QVariant computeValueAt(int rowIndex) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



class IndexCompositeColumn : public CompositeColumn {
	const QList<QPair<Column* const, Qt::SortOrder>> sorting;
	
public:
	IndexCompositeColumn(CompositeTable* table, QString uiName, const QList<QPair<Column* const, Qt::SortOrder>> sorting);
	
	virtual QVariant computeValueAt(int rowIndex) const override;
	QList<QVariant> computeWholeColumn() const override;
	QList<int> getRowIndexOrderList() const;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



#endif // COMPOSITE_COLUMN_H
