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

/**
 * @file comp_hikers_table.h
 * 
 * This file defines the CompositeHikersTable class.
 */

#ifndef COMP_HIKERS_TABLE_H
#define COMP_HIKERS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



/**
 * A subclass of CompositeTable for the user-facing hikers table.
 */
class CompositeHikersTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The name of the hiker. */
	const DirectCompositeColumn*	nameColumn;
	/** The number of ascents logged in which this hiker participated. */
	const FoldCompositeColumn*		numAscentsColumn;
	/** The number of trips logged in which this hiker participated. */
	const FoldCompositeColumn*		numTripsColumn;
	/** The average elevation gain of all ascents logged in which this hiker participated. */
	const FoldCompositeColumn*		avgElevationGainColumn;
	/** The maximum elevation gain of all ascents logged in which this hiker participated. */
	const FoldCompositeColumn*		maxElevationGainColumn;
	/** The sum of the elevation gains of all ascents logged in which this hiker participated. */
	const FoldCompositeColumn*		sumElevationGainColumn;
	
public:
	/**
	 * Creates a new CompositeHikersTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The hikers table view in the main window
	 */
	inline CompositeHikersTable(Database* db, QTableView* tableView) :
			CompositeTable(db, db->hikersTable, tableView),
			//																			uiName				align/fold op	suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn		(this,	tr("Name"),				Qt::AlignLeft,	noSuffix,	db->hikersTable->nameColumn)),
			numAscentsColumn		(new const NumericFoldCompositeColumn	(this,	tr("Num. ascents"),		CountFold,		noSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn} })),
			numTripsColumn			(new const NumericFoldCompositeColumn	(this,	tr("Num. trips"),		CountFold,		noSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn},	{db->ascentsTable->tripIDColumn,		db->tripsTable->primaryKeyColumn} })),
			avgElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	tr("Avg. elev. gain"),	AverageFold,	mSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn)),
			maxElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	tr("Max. elev. gain"),	MaxFold,		mSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn)),
			sumElevationGainColumn	(new const NumericFoldCompositeColumn	(this,	tr("Sum elev. gain"),	SumFold,		mSuffix,	{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn))
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
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {numAscentsColumn, Qt::DescendingOrder};
	}
};



#endif // COMP_HIKERS_TABLE_H
