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
 * @file comp_peaks_table.h
 * 
 * This file defines the CompositePeaksTable class.
 */

#ifndef COMP_PEAKS_TABLE_H
#define COMP_PEAKS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



/**
 * A subclass of CompositeTable for the user-facing peaks table.
 */
class CompositePeaksTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The name of the peak. */
	const DirectCompositeColumn			nameColumn;
	/** The height of the peak. */
	const DirectCompositeColumn			heightColumn;
	/** The name of the country the peak is assigned to. */
	const ReferenceCompositeColumn		countryColumn;
	/** The name of the region the peak is assigned to. */
	const ReferenceCompositeColumn		regionColumn;
	/** The name of the mountain range the peak is assigned to. */
	const ReferenceCompositeColumn		rangeColumn;
	/** The continent the peak is assigned to. */
	const ReferenceCompositeColumn		continentColumn;
	/** The number of ascents logged for this peak. */
	const NumericFoldCompositeColumn	numAscentsColumn;
	/** The names of all hikers who have ascents logged for this peak. */
	const HikerListFoldCompositeColumn	listHikersColumn;
	/** The sum of the elevation gains of all ascents logged for this peak. */
	const NumericFoldCompositeColumn	sumElevationGainColumn;
	/** Whether the peak is a volcano. */
	const DirectCompositeColumn			volcanoColumn;
	
public:
	/**
	 * Creates a new CompositePeaksTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The peaks table view in the main window
	 */
	inline CompositePeaksTable(Database& db, QTableView* tableView) :
		CompositeTable(db, db.peaksTable, tableView),
		//																name				uiName					suffix		fold op		content column / breadcrumbs
		nameColumn				(DirectCompositeColumn			(*this,												noSuffix,				db.peaksTable.nameColumn)),
		heightColumn			(DirectCompositeColumn			(*this,												mSuffix,				db.peaksTable.heightColumn)),
		countryColumn			(ReferenceCompositeColumn		(*this,	"country",			tr("Country"),			noSuffix,				db.countriesTable.nameColumn)),
		regionColumn			(ReferenceCompositeColumn		(*this,	"region",			tr("Region"),			noSuffix,				db.regionsTable.nameColumn)),
		rangeColumn				(ReferenceCompositeColumn		(*this,	"range",			tr("Mountain range"),	noSuffix,				db.rangesTable.nameColumn)),
		continentColumn			(ReferenceCompositeColumn		(*this,	"continent",		tr("Continent"),		noSuffix,				db.rangesTable.continentColumn)),
		numAscentsColumn		(NumericFoldCompositeColumn		(*this,	"numAscents",		tr("Num. ascents"),		noSuffix,	CountFold,	crumbsTo(db.ascentsTable))),
		listHikersColumn		(HikerListFoldCompositeColumn	(*this,	"listHikers",		tr("Scaled by"),								db.hikersTable.nameColumn)),
		sumElevationGainColumn	(NumericFoldCompositeColumn		(*this,	"sumElevationGain",	tr("Sum elev. gain"),	mSuffix,	SumFold,	db.ascentsTable.elevationGainColumn)),
		volcanoColumn			(DirectCompositeColumn			(*this,												noSuffix,				db.peaksTable.volcanoColumn))
	{
		addColumn(nameColumn);
		addColumn(heightColumn);
		addColumn(countryColumn);
		addColumn(regionColumn);
		addColumn(rangeColumn);
		addColumn(continentColumn);
		addColumn(numAscentsColumn);
		addColumn(listHikersColumn);
		addColumn(sumElevationGainColumn);
		addColumn(volcanoColumn);
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



#endif // COMP_PEAKS_TABLE_H
