/*
 * Copyright 2023-2024 Simon Vetter
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

/**
 * @file comp_trips_table.h
 * 
 * This file defines the CompositeTripsTable class.
 */

#ifndef COMP_TRIPS_TABLE_H
#define COMP_TRIPS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



/**
 * A subclass of CompositeTable for the user-facing trips table.
 */
class CompositeTripsTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The trip index numbers all trips chronologically and is always kept consistent with the data. */
	const IndexCompositeColumn*			indexColumn;
	/** The name of the trip. */
	const DirectCompositeColumn*		nameColumn;
	/** The start date of the trip. */
	const DirectCompositeColumn*		startDateColumn;
	/** The end date of the trip. */
	const DirectCompositeColumn*		endDateColumn;
	/** The length of the trip. */
	const DifferenceCompositeColumn*	lengthColumn;
	/** The number of ascents logged for this trip. */
	const FoldCompositeColumn*			numAscentsColumn;
	/** The average elevation gain of all ascents logged for this trip. */
	const FoldCompositeColumn*			avgElevationGainColumn;
	/** The maximum elevation gain of all ascents logged for this trip. */
	const FoldCompositeColumn*			maxElevationGainColumn;
	/** The sum of the elevation gains of all ascents logged for this trip. */
	const FoldCompositeColumn*			sumElevationGainColumn;
	/** The list of hikers who participated in any ascent logged for this trip. */
	const FoldCompositeColumn*			listHikersColumn;
	
public:
	/**
	 * Creates a new CompositeTripsTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The trips table view in the main window
	 */
	inline CompositeTripsTable(Database* db, QTableView* tableView) :
		CompositeTable(db, db->tripsTable, tableView),
		//																		name				uiName					suffix			fold op			[breadcrumbs +] content column
		indexColumn				(new const IndexCompositeColumn			(this,	"index",			tr("Index"),			noSuffix,						{ {db->tripsTable->startDateColumn,	Qt::AscendingOrder},					{db->tripsTable->endDateColumn,	Qt::AscendingOrder} })),
		nameColumn				(new const DirectCompositeColumn		(this,												noSuffix,						db->tripsTable->nameColumn)),
		startDateColumn			(new const DirectCompositeColumn		(this,												noSuffix,						db->tripsTable->startDateColumn)),
		endDateColumn			(new const DirectCompositeColumn		(this,												noSuffix,						db->tripsTable->endDateColumn)),
		lengthColumn			(new const DifferenceCompositeColumn	(this,	"length",			tr("Length"),			tr(" days"),					db->tripsTable->endDateColumn,		db->tripsTable->startDateColumn)),
		numAscentsColumn		(new const NumericFoldCompositeColumn	(this,	"numAscents",		tr("Num. ascents"),		noSuffix,		CountFold,		crumbsTo(db, db->ascentsTable))),
		avgElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	"avgElevationGain",	tr("Avg. elev. gain"),	mSuffix,		AverageFold,	crumbsTo(db, db->ascentsTable),		db->ascentsTable->elevationGainColumn)),
		maxElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	"maxElevationGain",	tr("Max. elev. gain"),	mSuffix,		MaxFold,		crumbsTo(db, db->ascentsTable),		db->ascentsTable->elevationGainColumn)),
		sumElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	"sumElevationGain",	tr("Sum elev. gain"),	mSuffix,		SumFold,		crumbsTo(db, db->ascentsTable),		db->ascentsTable->elevationGainColumn)),
		listHikersColumn		(new const HikerListFoldCompositeColumn	(this,	"listHikers",		tr("Participants"),										crumbsTo(db, db->hikersTable),		db->hikersTable->nameColumn))
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
	
	

	/**
	 * Returns the default sorting for the table.
	 *
	 * @return The default sorting for the table as a pair of the column to sort by and a Qt::SortOrder.
	 */
	virtual SortingPass getDefaultSorting() const override
	{
		return {indexColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_TRIPS_TABLE_H
