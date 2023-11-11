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
 * @file database.h
 * 
 * This file declares the Database class and the WhatIfDeleteResult struct.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "src/data/ascent.h"
#include "src/data/peak.h"
#include "src/data/trip.h"
#include "src/data/hiker.h"
#include "src/data/region.h"
#include "src/data/range.h"
#include "src/data/country.h"
#include "src/db/tables/ascents_table.h"
#include "src/db/tables/countries_table.h"
#include "src/db/tables/hikers_table.h"
#include "src/db/tables/participated_table.h"
#include "src/db/tables/peaks_table.h"
#include "src/db/tables/photos_table.h"
#include "src/db/tables/ranges_table.h"
#include "src/db/tables/regions_table.h"
#include "src/db/tables/trips_table.h"
#include "src/settings/project_settings.h"

#include <QStatusBar>
#include <QSqlQuery>
#include <QSqlError>

class MainWindow;
class CompositeAscentsTable;
struct WhatIfDeleteResult;



/**
 * A singleton class for accessing and manipulating the database.
 */
class Database {
	/** Whether a database is currently loaded. */
	bool databaseLoaded;
	/** The (functionally static) list of tables in any project database. */
	QList<Table*> tables;
	
	/** A pointer to the status bar of the main window, used to display status messages. */
	QStatusBar* mainWindowStatusBar;
	
public:
	/** The ascents table. */
	AscentsTable*		ascentsTable;
	/** The peaks table. */
	PeaksTable*			peaksTable;
	/** The trips table. */
	TripsTable*			tripsTable;
	/** The hikers table. */
	HikersTable*		hikersTable;
	/** The regions table. */
	RegionsTable*		regionsTable;
	/** The ranges table. */
	RangesTable*		rangesTable;
	/** The countries table. */
	CountriesTable*		countriesTable;
	/** The photos table. */
	PhotosTable*		photosTable;
	/** The participated table. */
	ParticipatedTable*	participatedTable;
	
	/** The project settings, based on a table. */
	ProjectSettings*	projectSettings;
	
	Database();
	~Database();
	
	void reset();
	void createNew(QWidget* parent, const QString& filepath);
	void openExisting(QWidget* parent, const QString& filepath);
	bool saveAs(QWidget* parent, const QString& filepath);
	QString getCurrentFilepath() const;
	
	QList<Table*> getTableList() const;
	
	Ascent*		getAscent	(ValidItemID ascentID)	const;
	Peak*		getPeak		(ValidItemID peakID)	const;
	Trip*		getTrip		(ValidItemID tripID)	const;
	Hiker*		getHiker	(ValidItemID hikerID)	const;
	Region*		getRegion	(ValidItemID regionID)	const;
	Range*		getRange	(ValidItemID rangeID)	const;
	Country*	getCountry	(ValidItemID countryID)	const;
	
	Ascent*		getAscentAt		(BufferRowIndex rowIndex) const;
	Peak*		getPeakAt		(BufferRowIndex rowIndex) const;
	Trip*		getTripAt		(BufferRowIndex rowIndex) const;
	Hiker*		getHikerAt		(BufferRowIndex rowIndex) const;
	Region*		getRegionAt		(BufferRowIndex rowIndex) const;
	Range*		getRangeAt		(BufferRowIndex rowIndex) const;
	Country*	getCountryAt	(BufferRowIndex rowIndex) const;
	
	QList<WhatIfDeleteResult> whatIf_removeRow(NormalTable* table, ValidItemID primaryKey);
	void removeRow(QWidget* parent, NormalTable* table, ValidItemID primaryKey);
private:
	QList<WhatIfDeleteResult> removeRow_referenceSearch(QWidget* parent, bool searchNotExecute, NormalTable* table, ValidItemID primaryKey);
	
	void populateBuffers(QWidget* parent, bool expectEmpty = false);
};



/**
 * A struct for storing a singular result of a what-if delete investigation.
 */
struct WhatIfDeleteResult {
	const Table*		affectedTable;
	const NormalTable*	itemTable;
	int					numAffectedRowIndices;
	
	WhatIfDeleteResult(const Table* affectedTable, const NormalTable* itemTable, int numAffectedRowIndices);
};



#endif // DATABASE_H
