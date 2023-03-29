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

#ifndef COMP_HIKERS_TABLE_H
#define COMP_HIKERS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeHikersTable : public CompositeTable {
	Q_OBJECT
	
public:
	const DirectCompositeColumn*	nameColumn;
	const FoldCompositeColumn*		numAscentsColumn;
	const FoldCompositeColumn*		numTripsColumn;
	const FoldCompositeColumn*		avgElevationGainColumn;
	const FoldCompositeColumn*		maxElevationGainColumn;
	const FoldCompositeColumn*		sumElevationGainColumn;
	
public:
	inline CompositeHikersTable(Database* db) :
			CompositeTable(db, db->hikersTable),
			//																		uiName				align/fold op	suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn	(this,	tr("Name"),				Qt::AlignLeft,	noSuffix,	db->hikersTable->nameColumn)),
			numAscentsColumn		(new const FoldCompositeColumn		(this,	tr("Num. ascents"),		Count,			noSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn} })),
			numTripsColumn			(new const FoldCompositeColumn		(this,	tr("Num. trips"),		Count,			noSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn},	{db->ascentsTable->tripIDColumn,		db->tripsTable->primaryKeyColumn} })),
			avgElevationGainColumn	(new const FoldCompositeColumn		(this,	tr("Avg. elev. gain"),	Average,		mSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn)),
			maxElevationGainColumn	(new const FoldCompositeColumn		(this,	tr("Max. elev. gain"),	Max,			mSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn)),
			sumElevationGainColumn	(new const FoldCompositeColumn		(this,	tr("Sum elev. gain"),	Sum,			mSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn))
	{
		addColumn(nameColumn);
		addColumn(numAscentsColumn);
		addColumn(numTripsColumn);
		addColumn(avgElevationGainColumn);
		addColumn(maxElevationGainColumn);
		addColumn(sumElevationGainColumn);
	}
	
	
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {numAscentsColumn, Qt::DescendingOrder};
	}
};



#endif // COMP_HIKERS_TABLE_H
