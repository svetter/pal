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
 * @file comp_ascents_table.h
 * 
 * This file defines the CompositeAscentsTable class.
 */

#ifndef COMP_ASCENTS_TABLE_H
#define COMP_ASCENTS_TABLE_H

#include "src/comp_tables/composite_table.h"
#include "src/comp_tables/fold_composite_column.h"



/**
 * A subclass of CompositeTable for the user-facing ascents table.
 */
class CompositeAscentsTable : public CompositeTable {
	Q_OBJECT
	
public:
	/** The ascent index numbers all ascents chronologically and is always kept consistent with the data. */
	const IndexCompositeColumn*			indexColumn;
	/** The date of the ascent. */
	const DirectCompositeColumn*		dateColumn;
	/** The name of the peak climbed. */
	const ReferenceCompositeColumn*		peakColumn;
	/** The title of the ascent. */
	const DirectCompositeColumn*		titleColumn;
	/** The height of the peak climbed. */
	const ReferenceCompositeColumn*		peakHeightColumn;
	/** The name of the country the peak is assigned to. */
	const ReferenceCompositeColumn*		countryColumn;
	/** The name of the region the peak is assigned to. */
	const ReferenceCompositeColumn*		regionColumn;
	/** The name of the mountain range the peak is assigned to. */
	const ReferenceCompositeColumn*		rangeColumn;
	/** The continent the peak is on. */
	const ReferenceCompositeColumn*		continentColumn;
	/** The elevation gain of the ascent. */
	const DirectCompositeColumn*		elevationGainColumn;
	/** The names of all hikers who participated in the ascent. */
	const FoldCompositeColumn*			hikersColumn;
	/** The name of the trip the ascent was part of. */
	const ReferenceCompositeColumn*		tripColumn;
	/** The kind of hike the ascent was. */
	const DirectCompositeColumn*		hikeKindColumn;
	/** Whether the ascent was a traverse. */
	const DirectCompositeColumn*		traverseColumn;
	/** The difficulty grade of the ascent. */
	const DependentEnumCompositeColumn*	difficultyColumn;
	/** Whether the peak climbed is a volcano. */
	const ReferenceCompositeColumn*		volcanoColumn;
	/** The peak ordinal indicates that this ascent was the nth one of the peak. */
	const OrdinalCompositeColumn*		peakOrdinalColumn;
	
	
	// === BACKEND COLUMNS ===
	
	// Export-only columns (unbuffered)
	const DirectCompositeColumn*		peakOnDayColumn;
	const DirectCompositeColumn*		timeColumn;
	const DirectCompositeColumn*		descriptionColumn;
	const ReferenceCompositeColumn*		tripStartDateColumn;
	const ReferenceCompositeColumn*		tripEndDateColumn;
	const ReferenceCompositeColumn*		tripDescriptionColumn;
	
	// Filter-only columns
	/** The ID of the mountain range the peak is assigned to. */
	const ReferenceCompositeColumn*		rangeIDColumn;
	/** The IDs of all hikers who participated in the ascent. */
	const FoldCompositeColumn*			hikerIDsColumn;
	
public:
	/**
	 * Creates a new CompositeAscentsTable.
	 * 
	 * Creates its own composite columns and adds them to the table.
	 * 
	 * @param db		The project database
	 * @param tableView	The ascents table view in the main window
	 */
	inline CompositeAscentsTable(const Database* db, QTableView* tableView) :
		CompositeTable(db, db->ascentsTable, tableView),
		//																		name				uiName						suffix		fold op		[breadcrumbs +] content column
		indexColumn				(new const IndexCompositeColumn			(this,	"index",			tr("Index"),				noSuffix,				{ {db->ascentsTable->dateColumn,			Qt::AscendingOrder},						{db->ascentsTable->peakOnDayColumn,		Qt::AscendingOrder},	{db->ascentsTable->timeColumn,	Qt::AscendingOrder} })),
		dateColumn				(new const DirectCompositeColumn		(this,													noSuffix,				db->ascentsTable->dateColumn)),
		peakColumn				(new const ReferenceCompositeColumn		(this,	"peak",				tr("Peak"),					noSuffix,				crumbsTo(db, db->peaksTable),				db->peaksTable->nameColumn)),
		titleColumn				(new const DirectCompositeColumn		(this,													noSuffix,				db->ascentsTable->titleColumn)),
		peakHeightColumn		(new const ReferenceCompositeColumn		(this,	"peakHeight",		tr("Height"),				mSuffix,				crumbsTo(db, db->peaksTable),				db->peaksTable->heightColumn)),
		countryColumn			(new const ReferenceCompositeColumn		(this,	"country",			tr("Country"),				noSuffix,				crumbsTo(db, db->countriesTable),			db->countriesTable->nameColumn)),
		regionColumn			(new const ReferenceCompositeColumn		(this,	"region",			tr("Region"),				noSuffix,				crumbsTo(db, db->regionsTable),				db->regionsTable->nameColumn)),
		rangeColumn				(new const ReferenceCompositeColumn		(this,	"range",			tr("Mountain range"),		noSuffix,				crumbsTo(db, db->rangesTable),				db->rangesTable->nameColumn)),
		continentColumn			(new const ReferenceCompositeColumn		(this,	"continent",		tr("Continent"),			noSuffix,				crumbsTo(db, db->rangesTable),				db->rangesTable->continentColumn)),
		elevationGainColumn		(new const DirectCompositeColumn		(this,													mSuffix,				db->ascentsTable->elevationGainColumn)),
		hikersColumn			(new const HikerListFoldCompositeColumn	(this,	"hikers",			tr("Participants"),									crumbsTo(db, db->hikersTable),				db->hikersTable->nameColumn)),
		tripColumn				(new const ReferenceCompositeColumn		(this,	"trip",				tr("Trip"),					noSuffix,				crumbsTo(db, db->tripsTable),				db->tripsTable->nameColumn)),
		hikeKindColumn			(new const DirectCompositeColumn		(this,													noSuffix,				db->ascentsTable->hikeKindColumn)),
		traverseColumn			(new const DirectCompositeColumn		(this,													noSuffix,				db->ascentsTable->traverseColumn)),
		difficultyColumn		(new const DependentEnumCompositeColumn	(this,	"difficulty",		tr("Difficulty"),									db->ascentsTable->difficultySystemColumn,	db->ascentsTable->difficultyGradeColumn)),
		volcanoColumn			(new const ReferenceCompositeColumn		(this,	"volcano",			tr("Volcano"),				noSuffix,				crumbsTo(db, db->peaksTable),				db->peaksTable->volcanoColumn)),
		peakOrdinalColumn		(new const OrdinalCompositeColumn		(this,	"peakOrdinal",		tr("Nth ascent of peak"),	".",					{ {db->ascentsTable->peakIDColumn,			Qt::AscendingOrder},						{db->ascentsTable->dateColumn,			Qt::AscendingOrder},	{db->ascentsTable->peakOnDayColumn,	Qt::AscendingOrder},	{db->ascentsTable->timeColumn,	Qt::AscendingOrder} })),
		
		// === BACKEND COLUMNS ===
		
		// Export-only columns (unbuffered)
		peakOnDayColumn			(new const DirectCompositeColumn		(this,													noSuffix,				db->ascentsTable->peakOnDayColumn)),
		timeColumn				(new const DirectCompositeColumn		(this,													noSuffix,				db->ascentsTable->timeColumn)),
		descriptionColumn		(new const DirectCompositeColumn		(this,													noSuffix,				db->ascentsTable->descriptionColumn)),
		tripStartDateColumn		(new const ReferenceCompositeColumn		(this,	"tripStartDate",	tr("Trip start date"),		noSuffix,				crumbsTo(db, db->tripsTable),				db->tripsTable->startDateColumn)),
		tripEndDateColumn		(new const ReferenceCompositeColumn		(this,	"tripEndDate",		tr("Trip end date"),		noSuffix,				crumbsTo(db, db->tripsTable),				db->tripsTable->endDateColumn)),
		tripDescriptionColumn	(new const ReferenceCompositeColumn		(this,	"tripDescription",	tr("Trip description"),		noSuffix,				crumbsTo(db, db->tripsTable),				db->tripsTable->descriptionColumn)),
		
		// Filter-only columns
		rangeIDColumn			(new const ReferenceCompositeColumn		(this,	"rangeID",			tr("Range ID"),				noSuffix,				crumbsTo(db, db->rangesTable),				db->rangesTable->primaryKeyColumn)),
		hikerIDsColumn			(new const NumericFoldCompositeColumn	(this,	"hikerIDs",			tr("Hiker IDs"),			noSuffix,	IDListFold,	crumbsTo(db, db->participatedTable),		db->participatedTable->hikerIDColumn))
	{
		addColumn(indexColumn);
		addColumn(dateColumn);
		addExportOnlyColumn(peakOnDayColumn);		// Export-only column
		addExportOnlyColumn(timeColumn);			// Export-only column
		addColumn(peakColumn);
		addColumn(titleColumn);
		addColumn(peakHeightColumn);
		addColumn(countryColumn);
		addColumn(regionColumn);
		addColumn(rangeColumn);
		addColumn(continentColumn);
		addColumn(elevationGainColumn);
		addColumn(hikersColumn);
		addColumn(tripColumn);
		addColumn(hikeKindColumn);
		addColumn(traverseColumn);
		addColumn(difficultyColumn);
		addColumn(volcanoColumn);
		addColumn(peakOrdinalColumn);
		addExportOnlyColumn(descriptionColumn);		// Export-only column
		addExportOnlyColumn(tripStartDateColumn);	// Export-only column
		addExportOnlyColumn(tripEndDateColumn);		// Export-only column
		addExportOnlyColumn(tripDescriptionColumn);	// Export-only column
		
		// Filter-only columns
		addFilterColumn(rangeIDColumn);
		addFilterColumn(hikerIDsColumn);
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



#endif // COMP_ASCENTS_TABLE_H
