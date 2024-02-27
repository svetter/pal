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
 * @file database.h
 * 
 * This file declares the Database class and the WhatIfDeleteResult struct.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "src/comp_tables/breadcrumbs.h"
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

#include <QSqlQuery>
#include <QSqlError>

using std::unique_ptr;

class MainWindow;
class CompositeAscentsTable;
struct WhatIfDeleteResult;



/**
 * A singleton class for accessing and manipulating the database.
 */
class Database {
	/** Whether a database is currently loaded. */
	bool databaseLoaded;
	/** The (functionally static) list of tables in any project database. Caution: Contains the project settings table! */
	QList<Table*> tables;
	
	/** A precomputed matrix of breadcrumb connections from any normal table to any other normal table in the project (settings table always excluded). */
	QMap<const NormalTable*, QMap<const NormalTable*, Breadcrumbs>> breadcrumbMatrix;
	
public:
	/** The trips table. */
	TripsTable			tripsTable;
	/** The hikers table. */
	HikersTable			hikersTable;
	/** The ranges table. */
	RangesTable			rangesTable;
	/** The countries table. */
	CountriesTable		countriesTable;
	/** The regions table. */
	RegionsTable		regionsTable;
	/** The peaks table. */
	PeaksTable			peaksTable;
	/** The ascents table. */
	AscentsTable		ascentsTable;
	/** The photos table. */
	PhotosTable			photosTable;
	/** The participated table. */
	ParticipatedTable	participatedTable;

	/** The project settings table. */
	SettingsTable		settingsTable;
	
	/** The project settings, based on a table. */
	ProjectSettings		projectSettings;
	
	Database();
	~Database();
	
	void reset();
	void createNew(QWidget& parent, const QString& filepath);
	bool openExisting(QWidget& parent, const QString& filepath);
	bool saveAs(QWidget& parent, const QString& filepath);
	QString getCurrentFilepath() const;
	
	QList<Table*> getItemTableList() const;
	QList<NormalTable*> getNormalItemTableList() const;
	
	unique_ptr<Ascent>	getAscent		(ValidItemID	ascentID)	const;
	unique_ptr<Peak>	getPeak			(ValidItemID	peakID)		const;
	unique_ptr<Trip>	getTrip			(ValidItemID	tripID)		const;
	unique_ptr<Hiker>	getHiker		(ValidItemID	hikerID)	const;
	unique_ptr<Region>	getRegion		(ValidItemID	regionID)	const;
	unique_ptr<Range>	getRange		(ValidItemID	rangeID)	const;
	unique_ptr<Country>	getCountry		(ValidItemID	countryID)	const;
	
	unique_ptr<Ascent>	getAscentAt		(BufferRowIndex	rowIndex)	const;
	unique_ptr<Peak>	getPeakAt		(BufferRowIndex	rowIndex)	const;
	unique_ptr<Trip>	getTripAt		(BufferRowIndex	rowIndex)	const;
	unique_ptr<Hiker>	getHikerAt		(BufferRowIndex	rowIndex)	const;
	unique_ptr<Region>	getRegionAt		(BufferRowIndex	rowIndex)	const;
	unique_ptr<Range>	getRangeAt		(BufferRowIndex	rowIndex)	const;
	unique_ptr<Country>	getCountryAt	(BufferRowIndex	rowIndex)	const;
	
	QList<WhatIfDeleteResult> whatIf_removeRows(NormalTable& table, QSet<ValidItemID> primaryKeys);
	void removeRows(QWidget& parent, NormalTable& table, QSet<ValidItemID> primaryKeys);
private:
	QList<WhatIfDeleteResult> removeRows_referenceSearch(QWidget* parent, bool searchNotExecute, NormalTable& table, QSet<ValidItemID> primaryKeys);
	
	void populateBuffers(QWidget& parent);
	
	void computeBreadcrumbMatrix();
public:
	Breadcrumbs getBreadcrumbsFor(const NormalTable& startTable, const NormalTable& targetTable) const;
	
	static QString tr(const QString& string);
	
	friend class DatabaseUpgrader;
};



/**
 * A struct for storing a singular result of a what-if delete investigation.
 */
struct WhatIfDeleteResult {
	const Table&		affectedTable;
	const NormalTable&	itemTable;
	int					numAffectedRowIndices;
	
	WhatIfDeleteResult(const Table& affectedTable, const NormalTable& itemTable, int numAffectedRowIndices);
};



#endif // DATABASE_H
