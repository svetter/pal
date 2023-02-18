#ifndef ASSOCIATIVE_TABLE_H
#define ASSOCIATIVE_TABLE_H

#include "table.h"



class AssociativeTable : public Table {
	const Column*	column1;
	const Column*	column2;
	
public:
	AssociativeTable(QString name, const Column* foreignKeyColumn1, const Column* foreignKeyColumn2);
	virtual ~AssociativeTable();
	
	const Column*	getColumn1() const;
	const Column*	getColumn2() const;
	const Column*	getOtherColumn(const Column* column) const;
	const Column*	getOwnColumnReferencing(const Column* column) const;
	const NormalTable* traverseAssociativeRelation(const Column* foreignColumn) const;
	
	int getNumberOfColumns() const override;
	QList<const Column*> getColumnList() const override;
	
	QSet<int> getMatchingEntries(const Column* column, int primaryKey) const;
	
	// Modifications
	void addRow(QWidget* parent, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const QList<QVariant>& primaryKeys);
};



#endif // ASSOCIATIVE_TABLE_H
