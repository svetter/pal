#ifndef PEAKS_TABLE_H
#define PEAKS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/peak.h"

#include <QWidget>



class PeaksTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* nameColumn;
	Column* heightColumn;
	Column* volcanoColumn;
	Column* regionIDColumn;
	Column* mapsLinkColumn;
	Column* earthLinkColumn;
	Column* wikiLinkColumn;
	
	PeaksTable(Column* foreignRegionIDColumn);
	
	int addRow(QWidget* parent, const Peak* peak);
};



#endif // PEAKS_TABLE_H
