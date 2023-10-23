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

#ifndef COMP_TRIPS_TABLE_H
#define COMP_TRIPS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



class CompositeTripsTable : public CompositeTable {
	Q_OBJECT
	
public:
	const IndexCompositeColumn*			indexColumn;
	const DirectCompositeColumn*		nameColumn;
	const DirectCompositeColumn*		startDateColumn;
	const DirectCompositeColumn*		endDateColumn;
	const DifferenceCompositeColumn*	lengthColumn;
	const FoldCompositeColumn*			numAscentsColumn;
	const FoldCompositeColumn*			avgElevationGainColumn;
	const FoldCompositeColumn*			maxElevationGainColumn;
	const FoldCompositeColumn*			sumElevationGainColumn;
	const FoldCompositeColumn*			listHikersColumn;
	
public:
	inline CompositeTripsTable(Database* db) :
			CompositeTable(db, db->tripsTable),
			//																			uiName				align/fold op	suffix			breadcrumbs (column reference chain) + content column [+ enum names]
			indexColumn				(new const IndexCompositeColumn			(this,	tr("Index"),							noSuffix,		{ {db->tripsTable->startDateColumn,		Qt::AscendingOrder},					{db->tripsTable->endDateColumn,				Qt::AscendingOrder} })),
			nameColumn				(new const DirectCompositeColumn		(this,	tr("Name"),				Qt::AlignLeft,	noSuffix,		db->tripsTable->nameColumn)),
			startDateColumn			(new const DirectCompositeColumn		(this,	tr("Start date"),		Qt::AlignLeft,	noSuffix,		db->tripsTable->startDateColumn)),
			endDateColumn			(new const DirectCompositeColumn		(this,	tr("End date"),			Qt::AlignLeft,	noSuffix,		db->tripsTable->endDateColumn)),
			lengthColumn			(new const DifferenceCompositeColumn	(this,	tr("Length"),							tr(" days"),	db->tripsTable->endDateColumn,			db->tripsTable->startDateColumn)),
			numAscentsColumn		(new const NumericFoldCompositeColumn	(this,	tr("Num. ascents"),		CountFold,		noSuffix,		{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} })),
			avgElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	tr("Avg. elev. gain"),	AverageFold,	mSuffix,		{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			maxElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	tr("Max. elev. gain"),	MaxFold,		mSuffix,		{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			sumElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	tr("Sum elev. gain"),	SumFold,		mSuffix,		{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			listHikersColumn		(new const HikerListCompositeColumn		(this,	tr("Participants"),										{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn},		{db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn},	{db->participatedTable->hikerIDColumn,	db->hikersTable->primaryKeyColumn} },	db->hikersTable->nameColumn))
	{
		addColumn(indexColumn);
		addColumn(nameColumn);
		addColumn(startDateColumn);
		addColumn(endDateColumn);
		addColumn(lengthColumn);
		addColumn(numAscentsColumn);
		addColumn(avgElevationGainColumn);
		addColumn(maxElevationGainColumn);
		addColumn(sumElevationGainColumn);
		addColumn(listHikersColumn);
	}
	
	
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {indexColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_TRIPS_TABLE_H
