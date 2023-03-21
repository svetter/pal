#ifndef COMP_RANGES_TABLE_H
#define COMP_RANGES_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeRangesTable : public CompositeTable {
	Q_OBJECT
	
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
			//																	uiName				align/fold op	suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn			(new const DirectCompositeColumn	(this,	tr("Mountain range"),	Qt::AlignLeft,	noSuffix,	db->rangesTable->nameColumn)),
			numRegionsColumn	(new const FoldCompositeColumn		(this,	tr("Num. regions"),		Count,			noSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn} })),
			numPeaksColumn		(new const FoldCompositeColumn		(this,	tr("Num. peaks"),		Count,			noSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Avg. peak height"),	Average,		mSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Max. peak height"),	Max,			mSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const FoldCompositeColumn		(this,	tr("Num. ascents"),		Count,			noSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
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
