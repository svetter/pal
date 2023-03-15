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
			//																	uiName				align/fold op	suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn			(new const DirectCompositeColumn	(this,	tr("Country"),			Qt::AlignLeft,	noSuffix,	db->countriesTable->nameColumn)),
			numRegionsColumn	(new const FoldCompositeColumn		(this,	tr("Num. regions"),		Count,			noSuffix,	{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn} })),
			numPeaksColumn		(new const FoldCompositeColumn		(this,	tr("Num. peaks"),		Count,			noSuffix,	{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Avg. peak height"),	Average,		mSuffix,	{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const FoldCompositeColumn		(this,	tr("Max. peak height"),	Max,			mSuffix,	{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const FoldCompositeColumn		(this,	tr("Num. ascents"),		Count,			noSuffix,	{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
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



#endif // COMP_COUNTRIES_TABLE_H
