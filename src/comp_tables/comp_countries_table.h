#ifndef COMP_COUNTRIES_TABLE_H
#define COMP_COUNTRIES_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeCountriesTable : public CompositeTable {
public:
	const DirectCompositeColumn*	nameColumn;
	const FoldCompositeColumn*		numRegionsColumn;
	const FoldCompositeColumn*		numPeaksColumn;
	const FoldCompositeColumn*		avgPeakHeightColumn;
	const FoldCompositeColumn*		maxPeakHeightColumn;
	const FoldCompositeColumn*		numAscentsColumn;
	
public:
	inline CompositeCountriesTable(Database* db) :
			CompositeTable(db, db->countriesTable),
			//															uiName					align/fold op	breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn			(new const DirectCompositeColumn	(tr("Country"),				Qt::AlignLeft,	db->countriesTable->nameColumn)),
			numRegionsColumn	(new const FoldCompositeColumn		(tr("Num. regions"),		Count,			{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn} })),
			numPeaksColumn		(new const FoldCompositeColumn		(tr("Num. peaks"),			Count,			{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const FoldCompositeColumn		(tr("Avg. peak height"),	Average,		{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const FoldCompositeColumn		(tr("Max. peak height"),	Max,			{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const FoldCompositeColumn		(tr("Num. ascents"),		Count,			{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
	{
		addColumn(nameColumn);
		addColumn(numRegionsColumn);
		addColumn(numPeaksColumn);
		addColumn(avgPeakHeightColumn);
		addColumn(maxPeakHeightColumn);
		addColumn(numAscentsColumn);
	}
};



#endif // COMP_COUNTRIES_TABLE_H
