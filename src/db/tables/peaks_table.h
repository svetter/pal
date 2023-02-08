#ifndef PEAKS_TABLE_H
#define PEAKS_TABLE_H

#include "src/db/db_model.h"
#include "src/data/peak.h"



class PeaksTable : public NormalTable {
public:
	Column* nameColumn;
	Column* heightColumn;
	Column* volcanoColumn;
	Column* regionIDColumn;
	Column* mapsLinkColumn;
	Column* earthLinkColumn;
	Column* wikiLinkColumn;
	
	PeaksTable(Column* foreignRegionIDColumn);
	
	int addRow(Peak* peak);
};



#endif // PEAKS_TABLE_H
