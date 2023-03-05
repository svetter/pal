#include "comp_regions_table.h"



CompositeRegionsTable::CompositeRegionsTable(Database* db) :
		CompositeTable(db, db->regionsTable),
		//															uiName
		regionColumn		(new const DirectCompositeColumn	(tr("Region"),				Qt::AlignLeft,	db->regionsTable->nameColumn)),
		rangeColumn			(new const ReferenceCompositeColumn	(tr("Mountain range"),		Qt::AlignLeft,	db->regionsTable->rangeIDColumn,		db->rangesTable->nameColumn)),
		countryColumn		(new const ReferenceCompositeColumn	(tr("Country"),				Qt::AlignLeft,	db->regionsTable->countryIDColumn,		db->countriesTable->nameColumn)),
		continentColumn		(new const ReferenceCompositeColumn	(tr("Continent"),			Qt::AlignLeft,	db->regionsTable->rangeIDColumn,		db->rangesTable->continentColumn)),
		numPeaksColumn		(new const FoldCompositeColumn		(tr("Num. peaks"),			Count,			{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
		avgPeakHeightColumn	(new const FoldCompositeColumn		(tr("Avg. peak height"),	Average,		{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
		maxPeakHeightColumn	(new const FoldCompositeColumn		(tr("Max. peak height"),	Max,			{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
		numAscentsColumn	(new const FoldCompositeColumn		(tr("Num. ascents"),		Count,			{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
{
	addColumn(regionColumn);
	addColumn(rangeColumn);
	addColumn(countryColumn);
	addColumn(continentColumn);
	addColumn(numPeaksColumn);
	addColumn(avgPeakHeightColumn);
	addColumn(maxPeakHeightColumn);
	addColumn(numAscentsColumn);
}
