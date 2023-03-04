#ifndef COMP_REGIONS_TABLE_H
#define COMP_REGIONS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeRegionsTable : public CompositeTable {
public:
	const CompositeColumn* regionColumn;
	const CompositeColumn* rangeColumn;
	const CompositeColumn* countryColumn;
	const CompositeColumn* continentColumn;
	const CompositeColumn* numPeaksColumn;
	const CompositeColumn* avgPeakHeightColumn;
	const CompositeColumn* maxPeakHeightColumn;
	const CompositeColumn* numAscentsColumn;
	
public:
	CompositeRegionsTable(Database* db);
};



#endif // COMP_REGIONS_TABLE_H
