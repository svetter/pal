#ifndef REGIONS_TABLE_H
#define REGIONS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/region.h"



class RegionsTable : public NormalTable {
public:
	const Column* nameColumn;
	const Column* rangeIDColumn;
	const Column* countryIDColumn;
	
	RegionsTable(const Column* foreignRangeIDColumn, const Column* foreignCountryIDColumn);
	
	int addRow(QWidget* parent, const Region* region);
};



#endif // REGIONS_TABLE_H
