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
	const DirectCompositeColumn*	nameColumn;
	/** The number of regions assigned to this mountain range. */
	const FoldCompositeColumn*		numRegionsColumn;
	/** The number of peaks assigned to this mountain range. */
	const FoldCompositeColumn*		numPeaksColumn;
	/** The average height of the peaks assigned to this mountain range. */
	const FoldCompositeColumn*		avgPeakHeightColumn;
	/** The maximum height of the peaks assigned to this mountain range. */
	const FoldCompositeColumn*		maxPeakHeightColumn;
	/** The number of ascents logged for this mountain range. */
	const FoldCompositeColumn*		numAscentsColumn;
	
public:
	/**
	 * Creates a new CompositeRangesTable.
	 *
	 * Creates its own composite columns and adds them to the table.
	 *
	 * @param db		The project database
	 * @param tableView	The ranges table view in the main window
	 */
	inline CompositeRangesTable(Database* db, QTableView* tableView) :
			CompositeTable(db, db->rangesTable, tableView),
			//																		uiName				align/fold op	suffix		breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn			(new const DirectCompositeColumn		(this,	tr("Mountain range"),	Qt::AlignLeft,	noSuffix,	db->rangesTable->nameColumn)),
			numRegionsColumn	(new const NumericFoldCompositeColumn	(this,	tr("Num. regions"),		CountFold,		noSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn} })),
			numPeaksColumn		(new const NumericFoldCompositeColumn	(this,	tr("Num. peaks"),		CountFold,		noSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} })),
			avgPeakHeightColumn	(new const NumericFoldCompositeColumn	(this,	tr("Avg. peak height"),	AverageFold,	mSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			maxPeakHeightColumn	(new const NumericFoldCompositeColumn	(this,	tr("Max. peak height"),	MaxFold,		mSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn} },	db->peaksTable->heightColumn)),
			numAscentsColumn	(new const NumericFoldCompositeColumn	(this,	tr("Num. ascents"),		CountFold,		noSuffix,	{ {db->rangesTable->primaryKeyColumn,	db->regionsTable->rangeIDColumn},		{db->regionsTable->primaryKeyColumn,	db->peaksTable->regionIDColumn},	{db->peaksTable->primaryKeyColumn,	db->ascentsTable->peakIDColumn} }))
	{
		addColumn(nameColumn);
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
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const override
	{
		return {nameColumn, Qt::AscendingOrder};
	}
};



#endif // COMP_RANGES_TABLE_H
