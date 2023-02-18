#ifndef PARTICIPATED_TABLE_H
#define PARTICIPATED_TABLE_H

#include "src/db/associative_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>


class ParticipatedTable : public AssociativeTable {
public:
	Column* ascentIDColumn;
	Column* hikerIDColumn;
	
	ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn);
	
	void addRows(QWidget* parent, Ascent* ascent);
	
private:
	void addRow(QWidget* parent, int ascentID, int hikerID);
	void removeRow(QWidget* parent, QPair<int, int>& primaryKey);
};



#endif // PARTICIPATED_TABLE_H
