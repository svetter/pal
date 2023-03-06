#ifndef COMP_REGIONS_TABLE_H
#define COMP_REGIONS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeRegionsTable : public CompositeTable {
public:
	const DirectCompositeColumn*	nameColumn;
	const ReferenceCompositeColumn*	rangeColumn;
	const ReferenceCompositeColumn*	countryColumn;
	const ReferenceCompositeColumn*	continentColumn;
	const FoldCompositeColumn*		numPeaksColumn;
	const FoldCompositeColumn*		avgPeakHeightColumn;
	const FoldCompositeColumn*		maxPeakHeightColumn;
	const FoldCompositeColumn*		numAscentsColumn;
	
public:
	inline CompositeRegionsTable(Database* db) :
			CompositeTable(db, db->regionsTable),
			//																	uiName				align/fold op	breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn			(new const DirectCompositeColumn	(this,	tr("Region"),			Qt::AlignLeft,	db->regionsTable->nameColumn)),
			rangeColumn			(new const ReferenceCompositeColumn	(this,	tr("Mountain range"),	Qt::AlignLeft,	{ db->regionsTable->rangeIDColumn },	db->rangesTable->nameColumn)),
			countryColumn		(new const ReferenceCompositeColumn	(this,	tr("Country"),			Qt::AlignLeft,	{ db->regionsTable->countryIDColumn },	db->countriesTable->nameColumn)),
			continentColumn		(new const ReferenceCompositeColumn	(this,	tr("Continent"),		Qt::AlignLeft,	{ db->regionsTable->rangeIDColumn },	db->rangesTable->continentColumn,	&Range::continentNames)),
			numPeaksColumn		(new const FoldCompositeColumn		(this,	tr("Num. peaks"),		Count,			{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Avg. peak height"),	Average,		{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Max. peak height"),	Max,			{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const FoldCompositeColumn		(this,	tr("Num. ascents"),		Count,			{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
	{
		addColumn(nameColumn);
		addColumn(rangeColumn);
		addColumn(countryColumn);
		addColumn(continentColumn);
		addColumn(numPeaksColumn);
		addColumn(avgPeakHeightColumn);
		addColumn(maxPeakHeightColumn);
		addColumn(numAscentsColumn);
	}
};



#endif // COMP_REGIONS_TABLE_H
