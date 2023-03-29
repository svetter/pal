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

#ifndef COMP_ASCENTS_TABLE_H
#define COMP_ASCENTS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeAscentsTable : public CompositeTable {
	Q_OBJECT
	
public:
	const DirectCompositeColumn*		dateColumn;
	const DirectCompositeColumn*		titleColumn;
	const ReferenceCompositeColumn*		peakColumn;
	const ReferenceCompositeColumn*		peakHeightColumn;
	const ReferenceCompositeColumn*		regionColumn;
	const ReferenceCompositeColumn*		rangeColumn;
	const ReferenceCompositeColumn*		countryColumn;
	const ReferenceCompositeColumn*		continentColumn;
	const ReferenceCompositeColumn*		volcanoColumn;
	const DirectCompositeColumn*		elevationGainColumn;
	const DirectCompositeColumn*		hikeKindColumn;
	const DirectCompositeColumn*		traverseColumn;
	const DependentEnumCompositeColumn*	difficultyColumn;
	const FoldCompositeColumn*			hikersColumn;
	const ReferenceCompositeColumn*		tripColumn;
	// Hidden (for filters)
	const ReferenceCompositeColumn*		rangeIDColumn;
	const FoldCompositeColumn*			hikerIDsColumn;
	
public:
	inline CompositeAscentsTable(Database* db) :
			CompositeTable(db, db->ascentsTable),
			//																		uiName				align/fold op		suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			dateColumn			(new const DirectCompositeColumn		(this,	tr("Date"),				Qt::AlignLeft,		noSuffix,	db->ascentsTable->dateColumn)),
			titleColumn			(new const DirectCompositeColumn		(this,	tr("Title"),			Qt::AlignLeft,		noSuffix,	db->ascentsTable->titleColumn)),
			peakColumn			(new const ReferenceCompositeColumn		(this,	tr("Peak"),				Qt::AlignLeft,		noSuffix,	{ db->ascentsTable->peakIDColumn },			db->peaksTable->nameColumn)),
			peakHeightColumn	(new const ReferenceCompositeColumn		(this,	tr("Height"),			Qt::AlignRight,		mSuffix,	{ db->ascentsTable->peakIDColumn },			db->peaksTable->heightColumn)),
			regionColumn		(new const ReferenceCompositeColumn		(this,	tr("Region"),			Qt::AlignLeft,		noSuffix,	{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn},			db->regionsTable->nameColumn)),
			rangeColumn			(new const ReferenceCompositeColumn		(this,	tr("Mountain range"),	Qt::AlignLeft,		noSuffix,	{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn,				db->regionsTable->rangeIDColumn},		db->rangesTable->nameColumn)),
			countryColumn		(new const ReferenceCompositeColumn		(this,	tr("Country"),			Qt::AlignLeft,		noSuffix,	{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn,				db->regionsTable->countryIDColumn},		db->countriesTable->nameColumn)),
			continentColumn		(new const ReferenceCompositeColumn		(this,	tr("Continent"),		Qt::AlignLeft,		noSuffix,	{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn,				db->regionsTable->rangeIDColumn},		db->rangesTable->continentColumn,		&Range::continentNames)),
			volcanoColumn		(new const ReferenceCompositeColumn		(this,	tr("Volcano"),			Qt::AlignCenter,	noSuffix,	{ db->ascentsTable->peakIDColumn },			db->peaksTable->volcanoColumn)),
			elevationGainColumn	(new const DirectCompositeColumn		(this,	tr("Elev. gain"),		Qt::AlignRight,		mSuffix,	db->ascentsTable->elevationGainColumn)),
			hikeKindColumn		(new const DirectCompositeColumn		(this,	tr("Kind of hike"),		Qt::AlignLeft,		noSuffix,	db->ascentsTable->hikeKindColumn,			&Ascent::hikeKindNames)),
			traverseColumn		(new const DirectCompositeColumn		(this,	tr("Traverse"),			Qt::AlignCenter,	noSuffix,	db->ascentsTable->traverseColumn)),
			difficultyColumn	(new const DependentEnumCompositeColumn	(this,	tr("Difficulty"),										db->ascentsTable->difficultySystemColumn,	db->ascentsTable->difficultyGradeColumn,	&Ascent::difficultyNames)),
			hikersColumn		(new const FoldCompositeColumn			(this,	tr("Participants"),		ListString,			noSuffix,	{ {db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn},		{db->participatedTable->hikerIDColumn,	db->hikersTable->primaryKeyColumn} },	db->hikersTable->nameColumn)),
			tripColumn			(new const ReferenceCompositeColumn		(this,	tr("Trip"),				Qt::AlignLeft,		noSuffix,	{ db->ascentsTable->tripIDColumn },			db->tripsTable->nameColumn)),
			// Hidden (for filters)
			rangeIDColumn		(new const ReferenceCompositeColumn		(this,	"Range ID",				Qt::AlignRight,		noSuffix,	{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn,				db->regionsTable->rangeIDColumn},		db->rangesTable->primaryKeyColumn)),
			hikerIDsColumn		(new const FoldCompositeColumn			(this,	"Hiker IDs",			IntList,			noSuffix,	{ {db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn} },	db->participatedTable->hikerIDColumn))
	{
		addColumn(dateColumn);
		addColumn(titleColumn);
		addColumn(peakColumn);
		addColumn(peakHeightColumn);
		addColumn(regionColumn);
		addColumn(rangeColumn);
		addColumn(countryColumn);
		addColumn(continentColumn);
		addColumn(volcanoColumn);
		addColumn(elevationGainColumn);
		addColumn(hikeKindColumn);
		addColumn(traverseColumn);
		addColumn(difficultyColumn);
		addColumn(hikersColumn);
		addColumn(tripColumn);
		// Hidden (for filters)
		addColumn(rangeIDColumn, true);
		addColumn(hikerIDsColumn, true);
	}
	
	
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {dateColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_ASCENTS_TABLE_H
