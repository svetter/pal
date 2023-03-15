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
			//																		uiName				align/fold op		suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn	(this,	tr("Peak"),				Qt::AlignLeft,		noSuffix,	db->peaksTable->nameColumn)),
			heightColumn			(new const DirectCompositeColumn	(this,	tr("Height"),			Qt::AlignRight,		mSuffix,	db->peaksTable->heightColumn)),
			volcanoColumn			(new const DirectCompositeColumn	(this,	tr("Volcano"),			Qt::AlignCenter,	noSuffix,	db->peaksTable->volcanoColumn)),
			regionColumn			(new const ReferenceCompositeColumn	(this,	tr("Region"),			Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn },		db->regionsTable->nameColumn)),
			rangeColumn				(new const ReferenceCompositeColumn	(this,	tr("Mountain range"),	Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn,		db->regionsTable->rangeIDColumn },		db->rangesTable->nameColumn)),
			countryColumn			(new const ReferenceCompositeColumn	(this,	tr("Country"),			Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn,		db->regionsTable->countryIDColumn },	db->countriesTable->nameColumn)),
			continentColumn			(new const ReferenceCompositeColumn	(this,	tr("Continent"),		Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn,		db->regionsTable->rangeIDColumn },		db->rangesTable->continentColumn,			&Range::continentNames)),
			numAscentsColumn		(new const FoldCompositeColumn		(this,	tr("Num. ascents"),		Count,				noSuffix,	{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} })),
			sumElevationGainColumn	(new const FoldCompositeColumn		(this,	tr("Sum elev. gain"),	Sum,				mSuffix,	{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} },		db->ascentsTable->elevationGainColumn)),
			listHikersColumn		(new const FoldCompositeColumn		(this,	tr("Scaled by"),		ListString,			noSuffix,	{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn},		{db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn},	{db->participatedTable->hikerIDColumn,	db->hikersTable->primaryKeyColumn} },	db->hikersTable->nameColumn))
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
	
	
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {nameColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_PEAKS_TABLE_H
