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
};



#endif // ASSOCIATIVE_TABLE_H
