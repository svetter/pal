#ifndef COMPOSITE_COLUMN_H
#define COMPOSITE_COLUMN_H

#include "src/comp_tables/filter.h"
#include "src/db/column.h"

#include <QString>
#include <QVariant>

class CompositeTable;



class CompositeColumn {
	CompositeTable* const table;
	
public:
	const QString uiName;
	const Qt::AlignmentFlag alignment;
	const DataType contentType;
	
protected:
	const QString suffix;
	const QStringList* const enumNames;
	const QList<QPair<QString, QStringList>>* const enumNameLists;
	
protected:
	CompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, QString suffix, const QStringList* enumNames = nullptr, const QList<QPair<QString, QStringList>>* enumNameLists = nullptr);
public:
	virtual ~CompositeColumn();
	
public:
	int getIndex() const;
	
	virtual QVariant computeValueAt(int rowIndex) const = 0;
	
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



#endif // COMPOSITE_COLUMN_H
