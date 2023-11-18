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
 * @file comp_regionss_table.h
 * 
 * This file defines the CompositeRegionsTable class.
 */

#ifndef COMP_REGIONS_TABLE_H
#define COMP_REGIONS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"
#include "src/data/enum_names.h"



/**
 * A subclass of CompositeTable for the user-facing regions table.
 */
class CompositeRegionsTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The name of the region. */
	const DirectCompositeColumn*	nameColumn;
	/** The name of the mountain range the region is assigned to. */
	const ReferenceCompositeColumn*	rangeColumn;
	/** The name of the country the region is assigned to. */
	const ReferenceCompositeColumn*	countryColumn;
	/** The continent the region is assigned to. */
	const ReferenceCompositeColumn*	continentColumn;
	/** The number of peaks assigned to this region. */
	const FoldCompositeColumn*		numPeaksColumn;
	/** The average height of all peaks assigned to this region. */
	const FoldCompositeColumn*		avgPeakHeightColumn;
	/** The height of the highest peak assigned to this region. */
	const FoldCompositeColumn*		maxPeakHeightColumn;
	/** The number of ascents assigned to this region. */
	const FoldCompositeColumn*		numAscentsColumn;
	
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
			//																	name				uiName					align/fold op	suffix		breadcrumbs (column reference chain) + content column
			nameColumn			(new const DirectCompositeColumn		(this,	"name",				tr("Region"),			Qt::AlignLeft,	noSuffix,	db->regionsTable->nameColumn)),
			rangeColumn			(new const ReferenceCompositeColumn		(this,	"range",			tr("Mountain range"),	Qt::AlignLeft,	noSuffix,	{ db->regionsTable->rangeIDColumn },	db->rangesTable->nameColumn)),
			countryColumn		(new const ReferenceCompositeColumn		(this,	"country",			tr("Country"),			Qt::AlignLeft,	noSuffix,	{ db->regionsTable->countryIDColumn },	db->countriesTable->nameColumn)),
			continentColumn		(new const ReferenceCompositeColumn		(this,	"continent",		tr("Continent"),		Qt::AlignLeft,	noSuffix,	{ db->regionsTable->rangeIDColumn },	db->rangesTable->continentColumn)),
			numPeaksColumn		(new const NumericFoldCompositeColumn	(this,	"numPeaks",			tr("Num. peaks"),		CountFold,		noSuffix,	{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const NumericFoldCompositeColumn	(this,	"avgPeakHeight",	tr("Avg. peak height"),	AverageFold,	mSuffix,	{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const NumericFoldCompositeColumn	(this,	"maxPeakHeight",	tr("Max. peak height"),	MaxFold,		mSuffix,	{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const NumericFoldCompositeColumn	(this,	"numAscents",		tr("Num. ascents"),		CountFold,		noSuffix,	{ {db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
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
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {nameColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_REGIONS_TABLE_H
