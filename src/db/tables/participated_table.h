#ifndef PARTICIPATED_TABLE_H
#define PARTICIPATED_TABLE_H

#include "src/db/db_model.h"
#include "src/data/ascent.h"

#include <QString>



class ParticipatedTable : public AssociativeTable {
public:
	ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn) :
			AssociativeTable(QString("Participated"), QString(), foreignAscentIDColumn, foreignHikerIDColumn)
	{}
	
	void addRows(Ascent* ascent);
};



#endif // PARTICIPATED_TABLE_H
