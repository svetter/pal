#ifndef HIKERS_TABLE_H
#define HIKERS_TABLE_H

#include "src/db/db_model.h"
#include "src/data/hiker.h"



class HikersTable : public NormalTable {
public:
	Column* nameColumn;
	
	HikersTable();
	
	int addRow(Hiker* hiker);
};



#endif // HIKERS_TABLE_H
