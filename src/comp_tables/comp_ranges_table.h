#ifndef COMP_RANGES_TABLE_H
#define COMP_RANGES_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeRangesTable : public CompositeTable {
public:
	const DirectCompositeColumn*	nameColumn;
	const FoldCompositeColumn*		numRegionsColumn;
	const FoldCompositeColumn*		numPeaksColumn;
	const FoldCompositeColumn*		avgPeakHeightColumn;
	const FoldCompositeColumn*		maxPeakHeightColumn;
	const FoldCompositeColumn*		numAscentsColumn;
	
public:
	inline CompositeRangesTable(Database* db) :
			CompositeTable(db, db->rangesTable),
			//															uiName					align/fold op	breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn			(new const DirectCompositeColumn	(tr("Mountain range"),		Qt::AlignLeft,	db->rangesTable->nameColumn)),
			numRegionsColumn	(new const FoldCompositeColumn		(tr("Num. regions"),		Count,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn} })),
			numPeaksColumn		(new const FoldCompositeColumn		(tr("Num. peaks"),			Count,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const FoldCompositeColumn		(tr("Avg. peak height"),	Average,		{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const FoldCompositeColumn		(tr("Max. peak height"),	Max,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const FoldCompositeColumn		(tr("Num. ascents"),		Count,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
	{
		addColumn(nameColumn);
		addColumn(numRegionsColumn);
		addColumn(numPeaksColumn);
		addColumn(avgPeakHeightColumn);
		addColumn(maxPeakHeightColumn);
		addColumn(numAscentsColumn);
	}
};



#endif // COMP_RANGES_TABLE_H
