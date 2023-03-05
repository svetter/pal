#ifndef COMP_PEAKS_TABLE_H
#define COMP_PEAKS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositePeaksTable : public CompositeTable {
public:
	const DirectCompositeColumn*	nameColumn;
	const DirectCompositeColumn*	heightColumn;
	const DirectCompositeColumn*	volcanoColumn;
	const ReferenceCompositeColumn*	regionColumn;
	const ReferenceCompositeColumn*	rangeColumn;
	const ReferenceCompositeColumn*	countryColumn;
	const ReferenceCompositeColumn*	continentColumn;
	const FoldCompositeColumn*		numAscentsColumn;
	const FoldCompositeColumn*		sumElevationGainColumn;
	const FoldCompositeColumn*		listHikersColumn;
	
public:
	inline CompositePeaksTable(Database* db) :
			CompositeTable(db, db->peaksTable),
			//																uiName				align/fold op		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn	(tr("Peak"),			Qt::AlignLeft,		db->peaksTable->nameColumn)),
			heightColumn			(new const DirectCompositeColumn	(tr("Height"),			Qt::AlignRight,		db->peaksTable->heightColumn)),
			volcanoColumn			(new const DirectCompositeColumn	(tr("Volcano"),			Qt::AlignCenter,	db->peaksTable->volcanoColumn)),
			regionColumn			(new const ReferenceCompositeColumn	(tr("Region"),			Qt::AlignLeft,		{ db->peaksTable->regionIDColumn },		db->regionsTable->nameColumn)),
			rangeColumn				(new const ReferenceCompositeColumn	(tr("Mountain range"),	Qt::AlignLeft,		{ db->peaksTable->regionIDColumn,		db->regionsTable->rangeIDColumn },		db->rangesTable->nameColumn)),
			countryColumn			(new const ReferenceCompositeColumn	(tr("Country"),			Qt::AlignLeft,		{ db->peaksTable->regionIDColumn,		db->regionsTable->countryIDColumn },	db->countriesTable->nameColumn)),
			continentColumn			(new const ReferenceCompositeColumn	(tr("Continent"),		Qt::AlignLeft,		{ db->peaksTable->regionIDColumn,		db->regionsTable->rangeIDColumn },		db->rangesTable->continentColumn,			&Range::continentNames)),
			numAscentsColumn		(new const FoldCompositeColumn		(tr("Num. ascents"),	Count,				{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} })),
			sumElevationGainColumn	(new const FoldCompositeColumn		(tr("Sum elev. gain"),	Sum,				{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} },		db->ascentsTable->elevationGainColumn)),
			listHikersColumn		(new const FoldCompositeColumn		(tr("Scaled by"),		List,				{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn},		{db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn},	{db->participatedTable->hikerIDColumn,	db->hikersTable->primaryKeyColumn} },	db->hikersTable->nameColumn))
	{
		addColumn(nameColumn);
		addColumn(heightColumn);
		addColumn(volcanoColumn);
		addColumn(regionColumn);
		addColumn(rangeColumn);
		addColumn(countryColumn);
		addColumn(continentColumn);
		addColumn(numAscentsColumn);
		addColumn(sumElevationGainColumn);
		addColumn(listHikersColumn);
	}
};



#endif // COMP_PEAKS_TABLE_H
