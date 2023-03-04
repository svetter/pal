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
	
public:
	virtual QVariant data(int rowIndex, int role) const = 0;
};



class DirectCompositeColumn : public CompositeColumn {
	const Column* baseColumn;
	
public:
	DirectCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, const Column* baseColumn);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



class ReferenceCompositeColumn : public CompositeColumn {
	const Column* keyColumn;
	const Column* referencedColumn;
	
public:
	ReferenceCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, const Column* keyColumn, const Column* referencedColumn);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



class CountCompositeColumn : public CompositeColumn {
	QList<QPair<const Column*, const Column*>> breadcrumbs;
	
public:
	CountCompositeColumn(QString uiName, QList<QPair<const Column*, const Column*>> breadcrumbs);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



enum CompositeColumnFoldOp {
	Count,
	List,
	Average,
	Sum,
	Max
};

class FoldCompositeColumn : public CompositeColumn {
	const CompositeColumnFoldOp op;
	const QList<QPair<const Column*, const Column*>> breadcrumbs;
	const Column* contentColumn;
	
public:
	FoldCompositeColumn(QString uiName, CompositeColumnFoldOp op, const QList<QPair<const Column*, const Column*>> breadcrumbs, const Column* contentColumn = nullptr);
	
	virtual QVariant data(int rowIndex, int role) const override;
};



#endif // COMPOSITE_COLUMN_H
