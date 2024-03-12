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
 * @file comp_hikers_table.h
 * 
 * This file defines the CompositeHikersTable class.
 */

#ifndef COMP_HIKERS_TABLE_H
#define COMP_HIKERS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"
#include "src/db/database.h"



/**
 * A subclass of CompositeTable for the user-facing hikers table.
 */
class CompositeHikersTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The name of the hiker. */
	const DirectCompositeColumn			nameColumn;
	/** The number of ascents logged in which this hiker participated. */
	const NumericFoldCompositeColumn	numAscentsColumn;
	/** The number of trips logged in which this hiker participated. */
	const NumericFoldCompositeColumn	numTripsColumn;
	/** The average elevation gain of all ascents logged in which this hiker participated. */
	const NumericFoldCompositeColumn	avgElevationGainColumn;
	/** The maximum elevation gain of all ascents logged in which this hiker participated. */
	const NumericFoldCompositeColumn	maxElevationGainColumn;
	/** The sum of the elevation gains of all ascents logged in which this hiker participated. */
	const NumericFoldCompositeColumn	sumElevationGainColumn;
	
public:
	/**
	 * Creates a new CompositeHikersTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The hikers table view in the main window
	 */
	inline CompositeHikersTable(Database& db, QTableView* tableView) :
		CompositeTable(db, db.hikersTable, tableView),
		//															name				uiName					suffix		fold op			content column / breadcrumbs
		nameColumn				(DirectCompositeColumn		(*this,												noSuffix,					db.hikersTable.nameColumn)),
		numAscentsColumn		(NumericFoldCompositeColumn	(*this,	"numAscents",		tr("Num. ascents"),		noSuffix,	CountFold,		crumbsTo(db.ascentsTable))),
		numTripsColumn			(NumericFoldCompositeColumn	(*this,	"numTrips",			tr("Num. trips"),		noSuffix,	CountFold,		crumbsTo(db.tripsTable))),
		avgElevationGainColumn	(NumericFoldCompositeColumn	(*this,	"avgElevationGain",	tr("Avg. elev. gain"),	mSuffix,	AverageFold,	db.ascentsTable.elevationGainColumn)),
		maxElevationGainColumn	(NumericFoldCompositeColumn	(*this,	"maxElevationGain",	tr("Max. elev. gain"),	mSuffix,	MaxFold,		db.ascentsTable.elevationGainColumn)),
		sumElevationGainColumn	(NumericFoldCompositeColumn	(*this,	"sumElevationGain",	tr("Sum elev. gain"),	mSuffix,	SumFold,		db.ascentsTable.elevationGainColumn))
	{
		addColumn(nameColumn);
		addColumn(numAscentsColumn);
		addColumn(numTripsColumn);
		addColumn(avgElevationGainColumn);
		addColumn(maxElevationGainColumn);
		addColumn(sumElevationGainColumn);
	}
	
	
	
	/**
	 * Returns the default sorting for the table.
	 *
	 * @return The default sorting for the table as a pair of the column to sort by and a Qt::SortOrder.
	 */
	virtual SortingPass getDefaultSorting() const override
	{
		return {&numAscentsColumn, Qt::DescendingOrder};
	}
};



#endif // COMP_HIKERS_TABLE_H
