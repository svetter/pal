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
			//																	uiName				align/fold op	breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn			(new const DirectCompositeColumn	(this,	tr("Mountain range"),	Qt::AlignLeft,	db->rangesTable->nameColumn)),
			numRegionsColumn	(new const FoldCompositeColumn		(this,	tr("Num. regions"),		Count,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn} })),
			numPeaksColumn		(new const FoldCompositeColumn		(this,	tr("Num. peaks"),		Count,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Avg. peak height"),	Average,		{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Max. peak height"),	Max,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const FoldCompositeColumn		(this,	tr("Num. ascents"),		Count,			{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
	{
		addColumn(nameColumn);
		addColumn(numRegionsColumn);
		addColumn(numPeaksColumn);
		addColumn(avgPeakHeightColumn);
		addColumn(maxPeakHeightColumn);
		addColumn(numAscentsColumn);
	}
	
	
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {nameColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_RANGES_TABLE_H
