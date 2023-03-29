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
