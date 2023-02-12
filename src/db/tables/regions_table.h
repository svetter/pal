#ifndef REGIONS_TABLE_H
#define REGIONS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/region.h"



class RegionsTable : public NormalTable {
public:
	Column* nameColumn;
	Column* rangeIDColumn;
	Column* countryIDColumn;
	
	RegionsTable(Column* foreignRangeIDColumn, Column* foreignCountryIDColumn);
	
	int addRow(Region* region);
};



#endif // REGIONS_TABLE_H
