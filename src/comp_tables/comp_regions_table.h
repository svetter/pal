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
 * @file comp_regionss_table.h
 * 
 * This file defines the CompositeRegionsTable class.
 */

#ifndef COMP_REGIONS_TABLE_H
#define COMP_REGIONS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



/**
 * A subclass of CompositeTable for the user-facing regions table.
 */
class CompositeRegionsTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The name of the region. */
	const DirectCompositeColumn			nameColumn;
	/** The name of the mountain range the region is assigned to. */
	const ReferenceCompositeColumn		rangeColumn;
	/** The name of the country the region is assigned to. */
	const ReferenceCompositeColumn		countryColumn;
	/** The continent the region is assigned to. */
	const ReferenceCompositeColumn		continentColumn;
	/** The number of peaks assigned to this region. */
	const NumericFoldCompositeColumn	numPeaksColumn;
	/** The average height of all peaks assigned to this region. */
	const NumericFoldCompositeColumn	avgPeakHeightColumn;
	/** The height of the highest peak assigned to this region. */
	const NumericFoldCompositeColumn	maxPeakHeightColumn;
	/** The number of ascents assigned to this region. */
	const NumericFoldCompositeColumn	numAscentsColumn;
	
public:
	/**
	 * Creates a new CompositeRegionsTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The regions table view in the main window
	 */
	inline CompositeRegionsTable(Database* db, QTableView* tableView) :
		CompositeTable(db, db->regionsTable, tableView),
		//														name				uiName					suffix		fold op			[breadcrumbs +] content column
		nameColumn			(DirectCompositeColumn		(this,												noSuffix,					db->regionsTable.nameColumn)),
		rangeColumn			(ReferenceCompositeColumn	(this,	"range",			tr("Mountain range"),	noSuffix,					crumbsTo(db, db->rangesTable),		db->rangesTable.nameColumn)),
		countryColumn		(ReferenceCompositeColumn	(this,	"country",			tr("Country"),			noSuffix,					crumbsTo(db, db->countriesTable),	db->countriesTable.nameColumn)),
		continentColumn		(ReferenceCompositeColumn	(this,	"continent",		tr("Continent"),		noSuffix,					crumbsTo(db, db->rangesTable),		db->rangesTable.continentColumn)),
		numPeaksColumn		(NumericFoldCompositeColumn	(this,	"numPeaks",			tr("Num. peaks"),		noSuffix,	CountFold,		crumbsTo(db, db->peaksTable))),
		avgPeakHeightColumn	(NumericFoldCompositeColumn	(this,	"avgPeakHeight",	tr("Avg. peak height"),	mSuffix,	AverageFold,	crumbsTo(db, db->peaksTable),		db->peaksTable.heightColumn)),
		maxPeakHeightColumn	(NumericFoldCompositeColumn	(this,	"maxPeakHeight",	tr("Max. peak height"),	mSuffix,	MaxFold,		crumbsTo(db, db->peaksTable),		db->peaksTable.heightColumn)),
		numAscentsColumn	(NumericFoldCompositeColumn	(this,	"numAscents",		tr("Num. ascents"),		noSuffix,	CountFold,		crumbsTo(db, db->ascentsTable)))
	{
		addColumn(nameColumn);
		addColumn(rangeColumn);
		addColumn(countryColumn);
		addColumn(continentColumn);
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



#endif // COMP_REGIONS_TABLE_H
