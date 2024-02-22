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
 * @file comp_ranges_table.h
 * 
 * This file defines the CompositeRangesTable class.
 */

#ifndef COMP_RANGES_TABLE_H
#define COMP_RANGES_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



/**
 * A subclass of CompositeTable for the user-facing ranges table.
 */
class CompositeRangesTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The name of the mountain range. */
	const DirectCompositeColumn			nameColumn;
	/** The continent the mountain range is on. */
	const DirectCompositeColumn			continentColumn;
	/** The number of regions assigned to this mountain range. */
	const NumericFoldCompositeColumn	numRegionsColumn;
	/** The number of peaks assigned to this mountain range. */
	const NumericFoldCompositeColumn	numPeaksColumn;
	/** The average height of the peaks assigned to this mountain range. */
	const NumericFoldCompositeColumn	avgPeakHeightColumn;
	/** The maximum height of the peaks assigned to this mountain range. */
	const NumericFoldCompositeColumn	maxPeakHeightColumn;
	/** The number of ascents logged for this mountain range. */
	const NumericFoldCompositeColumn	numAscentsColumn;
	
public:
	/**
	 * Creates a new CompositeRangesTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The ranges table view in the main window
	 */
	inline CompositeRangesTable(Database& db, QTableView* tableView) :
		CompositeTable(db, db.rangesTable, tableView),
		//														name				uiName					suffix		fold op			[breadcrumbs +] content column
		nameColumn			(DirectCompositeColumn		(*this,												noSuffix,					db.rangesTable.nameColumn)),
		continentColumn		(DirectCompositeColumn		(*this,												noSuffix,					db.rangesTable.continentColumn)),
		numRegionsColumn	(NumericFoldCompositeColumn	(*this,	"numRegions",		tr("Num. regions"),		noSuffix,	CountFold,		crumbsTo(db, db.regionsTable))),
		numPeaksColumn		(NumericFoldCompositeColumn	(*this,	"numPeaks",			tr("Num. peaks"),		noSuffix,	CountFold,		crumbsTo(db, db.peaksTable))),
		avgPeakHeightColumn	(NumericFoldCompositeColumn	(*this,	"avgPeakHeight",	tr("Avg. peak height"),	mSuffix,	AverageFold,	crumbsTo(db, db.peaksTable),		db.peaksTable.heightColumn)),
		maxPeakHeightColumn	(NumericFoldCompositeColumn	(*this,	"maxPeakHeight",	tr("Max. peak height"),	mSuffix,	MaxFold,		crumbsTo(db, db.peaksTable),		db.peaksTable.heightColumn)),
		numAscentsColumn	(NumericFoldCompositeColumn	(*this,	"numAscents",		tr("Num. ascents"),		noSuffix,	CountFold,		crumbsTo(db, db.ascentsTable)))
	{
		addColumn(nameColumn);
		addColumn(continentColumn);
		addColumn(numRegionsColumn);
		addColumn(numPeaksColumn);
		addColumn(avgPeakHeightColumn);
		addColumn(maxPeakHeightColumn);
		addColumn(numAscentsColumn);
	}
	
	
	
	/**
	 * Returns the default sorting for the table.
	 *
	 * @return The default sorting for the table as a pair of the column to sort by and a Qt::SortOrder.
	 */
	virtual SortingPass getDefaultSorting() const override
	{
		return {&nameColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_RANGES_TABLE_H
