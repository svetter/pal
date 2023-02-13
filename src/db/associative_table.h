#ifndef ASSOCIATIVE_TABLE_H
#define ASSOCIATIVE_TABLE_H

#include "table.h"



class AssociativeTable : public Table {
protected:
	Column*	column1;
	Column*	column2;
	
public:
	AssociativeTable(QString name, Column* foreignKeyColumn1, Column* foreignKeyColumn2);
	~AssociativeTable();
	
	Column* getColumn1();
	Column* getColumn2();
	
	int getNumberOfColumns() const override;
	QList<Column*> getColumnList() const override;
	
	QList<int> getMatchingEntries(Column* column, int primaryKey) const;
	
	// Modifications
	void addRow(QWidget* parent, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const QList<QVariant>& primaryKeys);
};



#endif // ASSOCIATIVE_TABLE_H
