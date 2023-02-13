#ifndef PARTICIPATED_TABLE_H
#define PARTICIPATED_TABLE_H

#include "src/db/associative_table.h"
#include "src/data/ascent.h"

#include <QString>



class ParticipatedTable : public AssociativeTable {
public:
	Column* ascentIDColumn;
	Column* hikerIDColumn;
	
	ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn);
	
	void addRows(Ascent* ascent);
};



#endif // PARTICIPATED_TABLE_H
