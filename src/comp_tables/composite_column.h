#ifndef COMPOSITE_COLUMN_H
#define COMPOSITE_COLUMN_H

#include "src/db/column.h"

#include <QString>
#include <QVariant>



class CompositeColumn {
public:
	const QString uiName;
	const Qt::AlignmentFlag alignment;
	
protected:
	CompositeColumn(QString uiName, Qt::AlignmentFlag alignment);
	
	static QVariant replaceEnumIfApplicable(QVariant content, const QStringList* enumNames);
	
public:
	virtual QVariant data(int rowIndex, int role) const = 0;
};



class DirectCompositeColumn : public CompositeColumn {
	const Column* contentColumn;
	const QStringList* enumNames;
	
public:
	DirectCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, const Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



class ReferenceCompositeColumn : public CompositeColumn {
	QList<const Column*> foreignKeyColumnSequence;
	const Column* contentColumn;
	const QStringList* enumNames;
	
public:
	ReferenceCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, QList<const Column*> foreignKeyColumnSequence, const Column* contentColumn, const QStringList* enumNames = nullptr);
	
	virtual QVariant data(int rowIndex, int role) const override;
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
	const QList<QPair<const Column*, const Column*>> breadcrumbs;
	const Column* contentColumn;
	const QStringList* enumNames;
	
public:
	FoldCompositeColumn(QString uiName, FoldOp op, const QList<QPair<const Column*, const Column*>> breadcrumbs, const Column* contentColumn = nullptr, const QStringList* enumNames = nullptr);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



class DifferenceCompositeColumn : public CompositeColumn {
	const Column* minuendColumn;
	const Column* subtrahendColumn;
	const QString suffix;
	
public:
	DifferenceCompositeColumn(QString uiName, const Column* minuendColumn, const Column* subtrahendColumn, const QString suffix);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



class DependentEnumCompositeColumn : public CompositeColumn {
	const Column* discerningEnumColumn;
	const Column* displayedEnumColumn;
	const QList<QPair<QString, QStringList>>* enumNameLists;
	
public:
	DependentEnumCompositeColumn(QString uiName, const Column* discerningEnumColumn, const Column* displayedEnumColumn, const QList<QPair<QString, QStringList>>* enumNameLists);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



#endif // COMPOSITE_COLUMN_H
