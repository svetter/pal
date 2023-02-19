#ifndef PARTICIPATED_TABLE_H
#define PARTICIPATED_TABLE_H

#include "src/db/associative_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>


class ParticipatedTable : public AssociativeTable {
public:
	const Column* ascentIDColumn;
	const Column* hikerIDColumn;
	
	ParticipatedTable(const Column* foreignAscentIDColumn, const Column* foreignHikerIDColumn);
	
	void addRows(QWidget* parent, const Ascent* ascent);
	
private:
	void addRow(QWidget* parent, ValidItemID ascentID, ValidItemID hikerID);
	void removeRow(QWidget* parent, QPair<int, int>& primaryKey);
};



#endif // PARTICIPATED_TABLE_H
