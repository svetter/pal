#ifndef PEAKS_TABLE_H
#define PEAKS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/peak.h"

#include <QWidget>



class PeaksTable : public NormalTable {
	Q_OBJECT
	
public:
	const Column* nameColumn;
	const Column* heightColumn;
	const Column* volcanoColumn;
	const Column* regionIDColumn;
	const Column* mapsLinkColumn;
	const Column* earthLinkColumn;
	const Column* wikiLinkColumn;
	
	PeaksTable(const Column* foreignRegionIDColumn);
	
	int addRow(QWidget* parent, const Peak* peak);
};



#endif // PEAKS_TABLE_H
