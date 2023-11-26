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
 * @file comp_countries_table.h
 * 
 * This file defines the CompositeCountriesTable class.
 */

#ifndef COMP_COUNTRIES_TABLE_H
#define COMP_COUNTRIES_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



/**
 * A subclass of CompositeTable for the user-facing countries table.
 */
class CompositeCountriesTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The name of the country. */
	const DirectCompositeColumn*	nameColumn;
	/** The number of ascents logged for this country. */
	const FoldCompositeColumn*		numAscentsColumn;
	/** The number of regions assigned to this country. */
	const FoldCompositeColumn*		numRegionsColumn;
	/** The number of peaks assigned to this country. */
	const FoldCompositeColumn*		numPeaksColumn;
	/** The average height of peaks assigned to this country. */
	const FoldCompositeColumn*		avgPeakHeightColumn;
	/** The height of the highest peak assigned to this country. */
	const FoldCompositeColumn*		maxPeakHeightColumn;
	
public:
	/**
	 * Creates a new CompositeCountriesTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The countries table view in the main window
	 */
	inline CompositeCountriesTable(Database* db, QTableView* tableView) :
		CompositeTable(db, db->countriesTable, tableView),
		//																	name				uiName					suffix		fold op			[breadcrumbs (column reference chain) +] content column
		nameColumn			(new const DirectCompositeColumn		(this,												noSuffix,					db->countriesTable->nameColumn)),
		numAscentsColumn	(new const NumericFoldCompositeColumn	(this,	"numAscents",		tr("Num. ascents"),		noSuffix,	CountFold,		{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} })),
		numRegionsColumn	(new const NumericFoldCompositeColumn	(this,	"numRegions",		tr("Num. regions"),		noSuffix,	CountFold,		{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn} })),
		numPeaksColumn		(new const NumericFoldCompositeColumn	(this,	"numPeaks",			tr("Num. peaks"),		noSuffix,	CountFold,		{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
		avgPeakHeightColumn	(new const NumericFoldCompositeColumn	(this,	"avgPeakHeight",	tr("Avg. peak height"),	mSuffix,	AverageFold,	{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
		maxPeakHeightColumn	(new const NumericFoldCompositeColumn	(this,	"maxPeakHeight",	tr("Max. peak height"),	mSuffix,	MaxFold,		{ {db->countriesTable->primaryKeyColumn,	db->regionsTable->countryIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn))
	{
		addColumn(nameColumn);
		addColumn(numAscentsColumn);
		addColumn(numRegionsColumn);
		addColumn(numPeaksColumn);
		addColumn(avgPeakHeightColumn);
		addColumn(maxPeakHeightColumn);
	}
	
	

	/**
	 * Returns the default sorting for the table.
	 *
	 * @return The default sorting for the table as a pair of the column to sort by and a Qt::SortOrder.
	 */
	virtual SortingPass getDefaultSorting() const override
	{
		return {nameColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_COUNTRIES_TABLE_H
