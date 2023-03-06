#ifndef ASSOCIATIVE_TABLE_H
#define ASSOCIATIVE_TABLE_H

#include "table.h"
#include "normal_table.h"

#include "src/data/item_id.h"



class AssociativeTable : public Table {
	Column* const column1;
	Column* const column2;
	
public:
	AssociativeTable(QString name, Column* foreignKeyColumn1, Column* foreignKeyColumn2);
	virtual ~AssociativeTable();
	
	// Column info
	Column* getColumn1() const;
	Column* getColumn2() const;
	const Column* getOtherColumn(const Column* column) const;
	const Column* getOwnColumnReferencing(const Column* column) const;
	const NormalTable* traverseAssociativeRelation(const Column* foreignColumn) const;
	
	// Buffer access
	int getNumberOfMatchingRows(const Column* column, ValidItemID primaryKey) const;
	QSet<ValidItemID> getMatchingEntries(const Column* column, ValidItemID primaryKey) const;
	
	// Modifications (passthrough)
	void addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const QList<const Column*>& primaryKeyColumns, const QList<ValidItemID>& primaryKeys);
	void removeMatchingRows(QWidget* parent, const Column* column, ValidItemID primaryKey);
	
	// QAbstractItemModel implementation (completes implementation in Table)
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
};



#endif // ASSOCIATIVE_TABLE_H
