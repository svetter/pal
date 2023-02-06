#ifndef RANGES_TABLE_H
#define RANGES_TABLE_H

#include "src/db/db_model.h"
#include "src/data/range.h"



class RangesTable : public NormalTable {
public:
	Column* nameColumn;
	Column* continentColumn;
	
	RangesTable();
	
	void addRow(Range* range);
};



#endif // RANGES_TABLE_H
