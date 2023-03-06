#ifndef COMPOSITE_COLUMN_H
#define COMPOSITE_COLUMN_H

#include "src/db/column.h"

#include <QString>
#include <QVariant>

class CompositeTable;



class CompositeColumn {
	CompositeTable* const table;
	
public:
	const QString uiName;
	const Qt::AlignmentFlag alignment;
	
protected:
	CompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment);
	
	static QVariant replaceEnumIfApplicable(QVariant content, const QStringList* enumNames);
	
public:
	virtual QVariant data(int rowIndex, int role) const = 0;
	
	int getIndexOf(const CompositeColumn* column) const;
	
	void announceChangedData() const;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const = 0;
};



class DirectCompositeColumn : public CompositeColumn {
	Column* const contentColumn;
	const QStringList* enumNames;
	
public:
	DirectCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant data(int rowIndex, int role) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



class ReferenceCompositeColumn : public CompositeColumn {
	QList<Column*> foreignKeyColumnSequence;
	Column* const contentColumn;
	const QStringList* enumNames;
	
public:
	ReferenceCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, QList<Column*> foreignKeyColumnSequence, Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant data(int rowIndex, int role) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



enum FoldOp {
	Count,
	List,
	Average,
	Sum,
	Max
};

class FoldCompositeColumn : public CompositeColumn {
	const FoldOp op;
	const QList<QPair<Column*, Column*>> breadcrumbs;
	Column* const contentColumn;
	const QStringList* enumNames;
	
public:
	FoldCompositeColumn(CompositeTable* table, QString uiName, FoldOp op, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn = nullptr, const QStringList* enumNames = nullptr);
	
	virtual QVariant data(int rowIndex, int role) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



class DifferenceCompositeColumn : public CompositeColumn {
	Column* const minuendColumn;
	Column* const subtrahendColumn;
	const QString suffix;
	
public:
	DifferenceCompositeColumn(CompositeTable* table, QString uiName, Column* minuendColumn, Column* subtrahendColumn, const QString suffix);
	
	virtual QVariant data(int rowIndex, int role) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



class DependentEnumCompositeColumn : public CompositeColumn {
	Column* const discerningEnumColumn;
	Column* const displayedEnumColumn;
	const QList<QPair<QString, QStringList>>* enumNameLists;
	
public:
	DependentEnumCompositeColumn(CompositeTable* table, QString uiName, Column* discerningEnumColumn, Column* displayedEnumColumn, const QList<QPair<QString, QStringList>>* enumNameLists);
	
	virtual QVariant data(int rowIndex, int role) const override;
	
	virtual const QSet<Column* const> getAllUnderlyingColumns() const override;
};



#endif // COMPOSITE_COLUMN_H
