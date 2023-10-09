/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMP_PEAKS_TABLE_H
#define COMP_PEAKS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositePeaksTable : public CompositeTable {
	Q_OBJECT
	
public:
	const DirectCompositeColumn*	nameColumn;
	const DirectCompositeColumn*	heightColumn;
	const ReferenceCompositeColumn*	countryColumn;
	const ReferenceCompositeColumn*	regionColumn;
	const ReferenceCompositeColumn*	rangeColumn;
	const ReferenceCompositeColumn*	continentColumn;
	const FoldCompositeColumn*		numAscentsColumn;
	const FoldCompositeColumn*		listHikersColumn;
	const FoldCompositeColumn*		sumElevationGainColumn;
	const DirectCompositeColumn*	volcanoColumn;
	
public:
	inline CompositePeaksTable(Database* db) :
			CompositeTable(db, db->peaksTable),
			//																		uiName				align/fold op		suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn	(this,	tr("Peak"),				Qt::AlignLeft,		noSuffix,	db->peaksTable->nameColumn)),
			heightColumn			(new const DirectCompositeColumn	(this,	tr("Height"),			Qt::AlignRight,		mSuffix,	db->peaksTable->heightColumn)),
			countryColumn			(new const ReferenceCompositeColumn	(this,	tr("Country"),			Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn,		db->regionsTable->countryIDColumn },	db->countriesTable->nameColumn)),
			regionColumn			(new const ReferenceCompositeColumn	(this,	tr("Region"),			Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn },		db->regionsTable->nameColumn)),
			rangeColumn				(new const ReferenceCompositeColumn	(this,	tr("Mountain range"),	Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn,		db->regionsTable->rangeIDColumn },		db->rangesTable->nameColumn)),
			continentColumn			(new const ReferenceCompositeColumn	(this,	tr("Continent"),		Qt::AlignLeft,		noSuffix,	{ db->peaksTable->regionIDColumn,		db->regionsTable->rangeIDColumn },		db->rangesTable->continentColumn,			&Range::continentNames)),
			numAscentsColumn		(new const FoldCompositeColumn		(this,	tr("Num. ascents"),		Count,				noSuffix,	{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} })),
			listHikersColumn		(new const FoldCompositeColumn		(this,	tr("Scaled by"),		ListString,			noSuffix,	{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn},		{db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn},	{db->participatedTable->hikerIDColumn,	db->hikersTable->primaryKeyColumn} },	db->hikersTable->nameColumn)),
			sumElevationGainColumn	(new const FoldCompositeColumn		(this,	tr("Sum elev. gain"),	Sum,				mSuffix,	{ {db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} },		db->ascentsTable->elevationGainColumn)),
			volcanoColumn			(new const DirectCompositeColumn	(this,	tr("Volcano"),			Qt::AlignCenter,	noSuffix,	db->peaksTable->volcanoColumn))
	{
		addColumn(nameColumn);
		addColumn(heightColumn);
		addColumn(countryColumn);
		addColumn(regionColumn);
		addColumn(rangeColumn);
		addColumn(continentColumn);
		addColumn(numAscentsColumn);
		addColumn(listHikersColumn);
		addColumn(sumElevationGainColumn);
		addColumn(volcanoColumn);
	}
	
	
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {nameColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_PEAKS_TABLE_H
