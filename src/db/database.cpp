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
 * @file database.cpp
 * 
 * This file defines the Database class and the WhatIfDeleteResult struct.
 */

#include "database.h"

#include "src/db/db_error.h"
#include "src/db/db_upgrade.h"
#include "src/main/helpers.h"

#include <QCoreApplication>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>

using std::unique_ptr, std::make_unique;



/**
 * Creates a new Database object, immediately creates the table models and initializes the SQLite
 * database driver.
 */
Database::Database() :
	databaseLoaded(false),
	tables(QList<Table*>()),
	breadcrumbMatrix(QMap<const NormalTable*, QMap<const NormalTable*, Breadcrumbs>>()),
	tripsTable			(TripsTable()),
	hikersTable			(HikersTable()),
	rangesTable			(RangesTable()),
	countriesTable		(CountriesTable()),
	regionsTable		(RegionsTable(rangesTable.primaryKeyColumn, countriesTable.primaryKeyColumn)),
	peaksTable			(PeaksTable(regionsTable.primaryKeyColumn)),
	ascentsTable		(AscentsTable(peaksTable.primaryKeyColumn, tripsTable.primaryKeyColumn)),
	photosTable			(PhotosTable(ascentsTable.primaryKeyColumn)),
	participatedTable	(ParticipatedTable(ascentsTable.primaryKeyColumn, hikersTable.primaryKeyColumn)),
	settingsTable		(SettingsTable()),
	projectSettings		(ProjectSettings(settingsTable))
{
	tables.append(&ascentsTable);
	tables.append(&peaksTable);
	tables.append(&tripsTable);
	tables.append(&hikersTable);
	tables.append(&regionsTable);
	tables.append(&rangesTable);
	tables.append(&countriesTable);
	tables.append(&photosTable);
	tables.append(&participatedTable);
	tables.append(&settingsTable);
	
	computeBreadcrumbMatrix();
	
	
	QSqlDatabase::addDatabase("QSQLITE");
}

/**
 * Destroys the Database object.
 */
Database::~Database()
{}



/**
 * Resets the database object and closes the connection to the database.
 * 
 * Used when closing a project.
 */
void Database::reset()
{
	for (Table* table : tables) {
		table->resetBuffer();
	}
	
	QSqlDatabase::database().close();
	
	databaseLoaded = false;
}

/**
 * Creates a new database file at the given filepath and initializes it.
 * 
 * @param parent	The parent window.
 * @param filepath	The filepath of the new database file.
 */
void Database::createNew(QWidget* parent, const QString& filepath)
{
	assert(!databaseLoaded);
	qDebug() << "Creating new database file at" << filepath;
	
	if (QFile(filepath).exists()) {
		qDebug() << "Existing file needs to be removed";
		QFile(filepath).remove();
	}
	
	// Set filename
	QSqlDatabase sql = QSqlDatabase::database();
	sql.setDatabaseName(filepath);
	
	// Open connection
	if (!sql.open()) {
		displayError(parent, sql.lastError());
	}
	databaseLoaded = true;
	
	qDebug() << "Creating tables in SQL";
	for (Table* table : tables) {
		table->createTableInSql(parent);
	}
	
	// All tables still empty of course, but this doubles as a table format check
	populateBuffers(parent);
	
	// Set version
	projectSettings.databaseVersion.set(parent, getAppVersion());
}

/**
 * Opens an existing database file at the given filepath and loads the data into the buffers.
 * 
 * @param parent	The parent window.
 * @param filepath	The filepath of the existing database file.
 * @return			True if the open was successful, false otherwise.
 */
bool Database::openExisting(QWidget* parent, const QString& filepath)
{
	assert(!databaseLoaded);
	qDebug() << "Opening database file" << filepath;
	
	// Set filename
	QSqlDatabase sql = QSqlDatabase::database();
	sql.setDatabaseName(filepath);
	
	// Open connection
	if (!sql.open()) {
		displayError(parent, sql.lastError());
	}
	databaseLoaded = true;
	
	// Upgrade database version
	DatabaseUpgrader upgrader = DatabaseUpgrader(*this, parent);
	bool abort = !upgrader.checkDatabaseVersionAndUpgrade([this, parent] () {
		// After database structure was updated if needed:
		populateBuffers(parent);
	});
	
	if (abort) {
		reset();
		return false;
	}
	return true;
}

/**
 * Copies the current database file to a new filepath and opens a connection to the new file.
 * 
 * @param parent	The parent window.
 * @return			True if the save was successful and the new file is now opened, false otherwise.
 */
bool Database::saveAs(QWidget* parent, const QString& filepath)
{
	assert(databaseLoaded);
	qDebug() << "Saving database file as" << filepath;
	
	QString oldFilepath = getCurrentFilepath();
	assert(!QFile(filepath).exists() && oldFilepath.compare(filepath, Qt:: CaseInsensitive) != 0);
	
	QSqlDatabase sql = QSqlDatabase::database();
	sql.close();
	
	// Copy file
	if (!QFile(oldFilepath).copy(filepath)) {
		qDebug() << "File copy failed:" << oldFilepath << "to" << filepath;
		
		// reopen old connection
		if (!sql.open())
			displayError(parent, sql.lastError());
		return false;
	}
	
	// Set filename
	sql.setDatabaseName(filepath);
	
	// Open connection
	if (!sql.open())
		displayError(parent, sql.lastError());
	
	return true;
}


/**
 * Returns the filepath of the currently open database file.
 * 
 * @pre A database file is currently open.
 * 
 * @return	The filepath of the currently open database file.
 */
QString Database::getCurrentFilepath() const
{
	assert(databaseLoaded);
	QSqlDatabase sql = QSqlDatabase::database();
	QString filepath = sql.databaseName();
	return filepath;
}


/**
 * Populates the buffers of all regular tables (not project settings) by loading the data from the
 * SQL database.
 * 
 * @pre A database file is currently open.
 * 
 * @param parent	The parent window.
 */
void Database::populateBuffers(QWidget* parent)
{
	assert(databaseLoaded);
	
	for (Table* table : tables) {
		assert(table->getNumberOfRows() == 0);
		table->initBuffer(parent);
	}
}



/**
 * For all pairs of normal tables (always excluding the project settings table), * computes the
 * breadcrumbs for the connection between them and stores them in the breadcrumbMatrix.
 * 
 * The algorithm consists of three steps:
 * 1.	Fill the diagonal with empty breadcrumbs (since connecting a table to itself does not
 * 		require any breadcrumbs).
 * 2.	Fill all trivial connections by iterating through all tables and adding breadcrumbs
 * 		consisting of a single column pair for each foreign key column.
 * 		This step adds two breadcrumbs for each table, one for either direction.
 * 		Associative tables are also included here, by adding connections between the two foreign
 * 		tables, traversing the associative table in either direction.
 * 3.	Iteratively find the remaining connections by chaining existing ones together. During this
 * 		process, the maximum length of accepted new breadcrumb chains is gradually increased to
 * 		prevent forming connections which turn back on themselves.
 */
void Database::computeBreadcrumbMatrix()
{
	const QList<Table*> tables = getItemTableList();
	const QList<NormalTable*> normalTables = getNormalItemTableList();
	const int numNormalTables = normalTables.size();
	const int numCells = numNormalTables * numNormalTables;
	int numFilled = 0;
	
	// Fill empty diagonal (normal table to itself)
	for (const NormalTable* const normalTable : normalTables) {
		breadcrumbMatrix[normalTable][normalTable] = Breadcrumbs();
		numFilled++;
	}
	
	// Fill trivial connections (one crumb or two over an associative table)
	for (const Table* const table : tables) {
		if (table->isAssociative) {
			// Associative table
			AssociativeTable* const associativeTable = (AssociativeTable*) table;
			PrimaryForeignKeyColumn& column1 = associativeTable->getColumn1();
			PrimaryForeignKeyColumn& column2 = associativeTable->getColumn2();
			PrimaryKeyColumn* const foreignColumn1 = column1.getReferencedForeignColumn();
			PrimaryKeyColumn* const foreignColumn2 = column2.getReferencedForeignColumn();
			const NormalTable* const foreignTable1 = (NormalTable*) foreignColumn1->table;
			const NormalTable* const foreignTable2 = (NormalTable*) foreignColumn2->table;
			assert(foreignColumn1 && foreignColumn2 && foreignTable1 && foreignTable2);
			
			assert(breadcrumbMatrix[foreignTable1][foreignTable2].isEmpty());
			assert(breadcrumbMatrix[foreignTable2][foreignTable1].isEmpty());
			
			// Foreign table to other foreign table via associative table (either side)
			breadcrumbMatrix[foreignTable1][foreignTable2] = Breadcrumbs({
				{*foreignColumn1,	column1},
				{column2,			*foreignColumn2}
			});
			breadcrumbMatrix[foreignTable2][foreignTable1] = Breadcrumbs({
				{*foreignColumn2,	column2},
				{column1,			*foreignColumn1}
			});
			numFilled += 2;
		}
		
		else {
			// Normal table
			const NormalTable* const normalTable = (NormalTable*) table;
			QList<const Column*> foreignKeyColumns = normalTable->getForeignKeyColumnList();
			for (const Column* const column : foreignKeyColumns) {
				ForeignKeyColumn* const localColumn = (ForeignKeyColumn*) column;
				PrimaryKeyColumn* const foreignColumn = column->getReferencedForeignColumn();
				const NormalTable* const foreignTable = (NormalTable*) foreignColumn->table;
				
				assert(breadcrumbMatrix[normalTable][foreignTable].isEmpty());
				assert(breadcrumbMatrix[foreignTable][normalTable].isEmpty());
				
				breadcrumbMatrix[normalTable][foreignTable] = Breadcrumbs({
					{*localColumn, *foreignColumn}
				});
				breadcrumbMatrix[foreignTable][normalTable] = Breadcrumbs({
					{*foreignColumn, *localColumn}
				});
				numFilled += 2;
			}
		}
	}
	
	// Iteratively find the remaining connections by chaining existing ones
	int connectionLengthLimit = 2;
	// Gradually increase the maximum length of combined connections which are accepted to prevent
	// ending up with connections which turn back on themselves.
	while (numFilled < numCells) {
		const int numFilledBeforeIter = numFilled;
		
		for (const NormalTable* const tableA : normalTables) {
			for (const NormalTable* const tableB : normalTables) {
				if (tableA == tableB) continue;
				if (breadcrumbMatrix[tableA][tableB].isEmpty()) continue;
				// We are on a connection A -> B which has already been found.
				// We are now looking for another existing connection B -> C. We then derive A -> C.
				
				for (const NormalTable* const tableC : normalTables) {
					if (tableB == tableC || tableA == tableC) continue;
					if (breadcrumbMatrix[tableB][tableC].isEmpty()) continue;
					// We have found an existing connection B -> C.
					if (!breadcrumbMatrix[tableA][tableC].isEmpty()) continue;
					
					const Breadcrumbs& breadcrumbsAtoB = breadcrumbMatrix[tableA][tableB];
					const Breadcrumbs& breadcrumbsBtoC = breadcrumbMatrix[tableB][tableC];
					
					int candidateLength = breadcrumbsAtoB.length() + breadcrumbsBtoC.length();
					if (candidateLength > connectionLengthLimit) continue;
					
					breadcrumbMatrix[tableA][tableC] = breadcrumbsAtoB + breadcrumbsBtoC;
					numFilled++;
				}
			}
		}
		
		assert(numFilled > numFilledBeforeIter);
		connectionLengthLimit++;
	}
}

/**
 * Returns the pre-computed breadcrumbs for the connection between the given normal tables.
 * 
 * @param startTable	The table to start from (e.g. where the user made a selection).
 * @param targetTable	The table which must be reached.
 * @return				The breadcrumbs for the connection between the given tables.
 */
Breadcrumbs Database::getBreadcrumbsFor(const NormalTable& startTable, const NormalTable& targetTable) const
{
	assert(breadcrumbMatrix.contains(&startTable));
	assert(breadcrumbMatrix.value(&startTable).contains(&targetTable));
	
	return breadcrumbMatrix.value(&startTable).value(&targetTable);
}



/**
 * Translates the given string under the context "Database".
 * 
 * @param string	The string to translate.
 * @return			The translated string.
 */
QString Database::tr(const QString& string)
{
	return QCoreApplication::translate("Database", string.toStdString().c_str());
}



/**
 * Returns a list of all tables in the database (not including the project settings table).
 * 
 * @return	A list of all regular tables (not project settings) in the database.
 */
QList<Table*> Database::getItemTableList() const
{
	QList<Table*> list = QList<Table*>();
	for (Table* const table : tables) {
		if (table == &settingsTable) continue;
		list.append(table);
	}
	return list;
}

/**
 * Returns a list of all normal tables in the database (not including the project settings table).
 * 
 * @return	A list of all regular normal tables (not project settings) in the database.
 */
QList<NormalTable*> Database::getNormalItemTableList() const
{
	QList<NormalTable*> list = QList<NormalTable*>();
	for (const Table* const table : tables) {
		if (table == &settingsTable) continue;
		if (table->isAssociative) continue;
		list.append((NormalTable*) table);
	}
	return list;
}



/**
 * Fetches the data for the ascent with the given ID from the database and returns it as an Ascent
 * object.
 * 
 * @pre A database file is currently open.
 * 
 * @param ascentID	The ID of the ascent to fetch.
 * @return			The ascent with the given ID.
 */
unique_ptr<Ascent> Database::getAscent(ValidItemID ascentID) const
{
	assert(databaseLoaded);
	return getAscentAt(ascentsTable.getBufferIndexForPrimaryKey(ascentID));
}

/**
 * Fetches the data for the peak with the given ID from the database and returns it as a Peak
 * object.
 * 
 * @pre A database file is currently open.
 * 
 * @param peakID	The ID of the peak to fetch.
 * @return			The peak with the given ID.
 */
unique_ptr<Peak> Database::getPeak(ValidItemID peakID) const
{
	assert(databaseLoaded);
	return getPeakAt(peaksTable.getBufferIndexForPrimaryKey(peakID));
}

/**
 * Fetches the data for the trip with the given ID from the database and returns it as a Trip
 * object.
 * 
 * @pre A database file is currently open.
 * 
 * @param tripID	The ID of the trip to fetch.
 * @return			The trip with the given ID.
 */
unique_ptr<Trip> Database::getTrip(ValidItemID tripID) const
{
	assert(databaseLoaded);
	return getTripAt(tripsTable.getBufferIndexForPrimaryKey(tripID));
}

/**
 * Fetches the data for the hiker with the given ID from the database and returns it as a Hiker
 * object.
 * 
 * @pre A database file is currently open.
 * 
 * @param hikerID	The ID of the hiker to fetch.
 * @return			The hiker with the given ID.
 */
unique_ptr<Hiker> Database::getHiker(ValidItemID hikerID) const
{
	assert(databaseLoaded);
	return getHikerAt(hikersTable.getBufferIndexForPrimaryKey(hikerID));
}

/**
 * Fetches the data for the region with the given ID from the database and returns it as a Region
 * object.
 * 
 * @pre A database file is currently open.
 * 
 * @param regionID	The ID of the region to fetch.
 * @return			The region with the given ID.
 */
unique_ptr<Region> Database::getRegion(ValidItemID regionID) const
{
	assert(databaseLoaded);
	return getRegionAt(regionsTable.getBufferIndexForPrimaryKey(regionID));
}

/**
 * Fetches the data for the range with the given ID from the database and returns it as a Range
 * object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rangeID	The ID of the range to fetch.
 * @return			The range with the given ID.
 */
unique_ptr<Range> Database::getRange(ValidItemID rangeID) const
{
	assert(databaseLoaded);
	return getRangeAt(rangesTable.getBufferIndexForPrimaryKey(rangeID));
}

/**
 * Fetches the data for the country with the given ID from the database and returns it as a
 * Country object.
 * 
 * @pre A database file is currently open.
 * 
 * @param countryID	The ID of the country to fetch.
 * @return			The country with the given ID.
 */
unique_ptr<Country> Database::getCountry(ValidItemID countryID) const
{
	assert(databaseLoaded);
	return getCountryAt(countriesTable.getBufferIndexForPrimaryKey(countryID));
}



/**
 * Fetches the data for the ascent at the given buffer row index from the database and returns it
 * as an Ascent object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rowIndex	The buffer row index of the ascent to fetch.
 * @return			The ascent at the given buffer row index.
 */
unique_ptr<Ascent> Database::getAscentAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = ascentsTable.getBufferRow(rowIndex);
	assert(row->size() == ascentsTable.getNumberOfColumns());
	
	ValidItemID ascentID = VALID_ITEM_ID(row->at(ascentsTable.primaryKeyColumn.getIndex()));
	QString	title				= row->at(ascentsTable.titleColumn				.getIndex()).toString();
	ItemID	peakID				= row->at(ascentsTable.peakIDColumn				.getIndex());
	QDate	date				= row->at(ascentsTable.dateColumn				.getIndex()).toDate();
	int		perDayIndex			= row->at(ascentsTable.peakOnDayColumn			.getIndex()).toInt();
	QTime	time				= row->at(ascentsTable.timeColumn				.getIndex()).toTime();
	int		elevationGain		= row->at(ascentsTable.elevationGainColumn		.getIndex()).toInt();
	int		hikeKind			= row->at(ascentsTable.hikeKindColumn			.getIndex()).toInt();
	bool	traverse			= row->at(ascentsTable.traverseColumn			.getIndex()).toBool();
	int		difficultySystem	= row->at(ascentsTable.difficultySystemColumn	.getIndex()).toInt();
	int		difficultyGrade		= row->at(ascentsTable.difficultyGradeColumn	.getIndex()).toInt();
	ItemID	tripID				= row->at(ascentsTable.tripIDColumn				.getIndex());
	QString	description			= row->at(ascentsTable.descriptionColumn		.getIndex()).toString();
	
	QSet<ValidItemID>	hikerIDs	= participatedTable.getMatchingEntries(participatedTable.ascentIDColumn, ascentID);
	QList<Photo>		photos		= photosTable.getPhotosForAscent(ascentID);
	
	return make_unique<Ascent>(ascentID, title, peakID, date, perDayIndex, time, elevationGain, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, hikerIDs, photos, description);
}

/**
 * Fetches the data for the peak at the given buffer row index from the database and returns it as
 * a Peak object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rowIndex	The buffer row index of the peak to fetch.
 * @return			The peak at the given buffer row index.
 */
unique_ptr<Peak> Database::getPeakAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = peaksTable.getBufferRow(rowIndex);
	assert(row->size() == peaksTable.getNumberOfColumns());
	
	ValidItemID peakID = VALID_ITEM_ID(row->at(peaksTable.primaryKeyColumn	.getIndex()));
	QString	name		= row->at(peaksTable.nameColumn			.getIndex()).toString();
	int		height		= row->at(peaksTable.heightColumn		.getIndex()).toInt();
	bool	volcano		= row->at(peaksTable.volcanoColumn		.getIndex()).toBool();
	ItemID	regionID	= row->at(peaksTable.regionIDColumn		.getIndex()).toInt();
	QString	mapsLink	= row->at(peaksTable.mapsLinkColumn		.getIndex()).toString();
	QString	earthLink	= row->at(peaksTable.earthLinkColumn	.getIndex()).toString();
	QString	wikiLink	= row->at(peaksTable.wikiLinkColumn		.getIndex()).toString();
	
	return make_unique<Peak>(peakID, name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
}

/**
 * Fetches the data for the trip at the given buffer row index from the database and returns it as
 * a Trip object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rowIndex	The buffer row index of the trip to fetch.
 * @return			The trip at the given buffer row index.
 */
unique_ptr<Trip> Database::getTripAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = tripsTable.getBufferRow(rowIndex);
	assert(row->size() == tripsTable.getNumberOfColumns());
	
	ValidItemID tripID = VALID_ITEM_ID(row->at(tripsTable.primaryKeyColumn.getIndex()));
	QString	name		= row->at(tripsTable.nameColumn			.getIndex()).toString();
	QDate	startDate	= row->at(tripsTable.startDateColumn	.getIndex()).toDate();
	QDate	endDate		= row->at(tripsTable.endDateColumn		.getIndex()).toDate();
	QString	description	= row->at(tripsTable.descriptionColumn	.getIndex()).toString();
	
	return make_unique<Trip>(tripID, name, startDate, endDate, description);
}

/**
 * Fetches the data for the hiker at the given buffer row index from the database and returns it
 * as a Hiker object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rowIndex	The buffer row index of the hiker to fetch.
 * @return			The hiker at the given buffer row index.
 */
unique_ptr<Hiker> Database::getHikerAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = hikersTable.getBufferRow(rowIndex);
	assert(row->size() == hikersTable.getNumberOfColumns());
	
	ValidItemID hikerID = VALID_ITEM_ID(row->at(hikersTable.primaryKeyColumn.getIndex()));
	QString	name	= row->at(hikersTable.nameColumn.getIndex()).toString();
	
	return make_unique<Hiker>(hikerID, name);
}

/**
 * Fetches the data for the region at the given buffer row index from the database and returns it
 * as a Region object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rowIndex	The buffer row index of the region to fetch.
 * @return			The region at the given buffer row index.
 */
unique_ptr<Region> Database::getRegionAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = regionsTable.getBufferRow(rowIndex);
	assert(row->size() == regionsTable.getNumberOfColumns());
	
	ValidItemID regionID = VALID_ITEM_ID(row->at(regionsTable.primaryKeyColumn.getIndex()));
	QString	name		= row->at(regionsTable.nameColumn		.getIndex()).toString();
	ItemID	rangeID		= row->at(regionsTable.rangeIDColumn	.getIndex()).toInt();
	ItemID	countryID	= row->at(regionsTable.countryIDColumn	.getIndex()).toInt();
	
	return make_unique<Region>(regionID, name, rangeID, countryID);
}

/**
 * Fetches the data for the range at the given buffer row index from the database and returns it
 * as a Range object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rowIndex	The buffer row index of the range to fetch.
 * @return			The range at the given buffer row index.
 */
unique_ptr<Range> Database::getRangeAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = rangesTable.getBufferRow(rowIndex);
	assert(row->size() == rangesTable.getNumberOfColumns());
	
	ValidItemID rangeID = VALID_ITEM_ID(row->at(rangesTable.primaryKeyColumn.getIndex()));
	QString	name		= row->at(rangesTable.nameColumn		.getIndex()).toString();
	int		continent	= row->at(rangesTable.continentColumn	.getIndex()).toInt();
	
	return make_unique<Range>(rangeID, name, continent);
}

/**
 * Fetches the data for the country at the given buffer row index from the database and returns it
 * as a Country object.
 * 
 * @pre A database file is currently open.
 * 
 * @param rowIndex	The buffer row index of the country to fetch.
 * @return			The country at the given buffer row index.
 */
unique_ptr<Country> Database::getCountryAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = countriesTable.getBufferRow(rowIndex);
	assert(row->size() == countriesTable.getNumberOfColumns());
	
	ValidItemID countryID = VALID_ITEM_ID(row->at(countriesTable.primaryKeyColumn.getIndex()));
	QString	name	= row->at(countriesTable.nameColumn.getIndex()).toString();
	
	return make_unique<Country>(countryID, name);
}



/**
 * Returns a list of consequences of removing the rows with the given primary keys from the given
 * normal table.
 * 
 * A "consequence" of a deletion here is any instance where another item would have information
 * associated with it change as a result of the deletion. For example, if a peak is deleted, all
 * ascents of that peak would now be without a peak, so they would be counted as a consequence.
 * All consequences of the same kind, i.e., affected items in the same table, are grouped together
 * as one WhatIfDeleteResult with a count of how many items would be affected.
 * 
 * Delegates to removeRows_referenceSearch().
 * 
 * @see WhatIfDeleteResult
 * 
 * @pre A database file is currently open.
 * 
 * @param table			The table to search in.
 * @param primaryKeys	The primary keys to search for.
 * @return				A list of all rows in the given table that reference the given primary key.
 */
QList<WhatIfDeleteResult> Database::whatIf_removeRows(NormalTable& table, QSet<ValidItemID> primaryKeys)
{
	assert(databaseLoaded);
	
	return Database::removeRows_referenceSearch(nullptr, true, table, primaryKeys);
}

/**
 * Removes the rows with the given primary keys from the given normal table.
 * 
 * Before removing the row, all references to the row have to be removed from other tables. This
 * task is delegated to removeRows_referenceSearch().
 * 
 * @pre A database file is currently open.
 * 
 * @param parent		The parent window.
 * @param table			The table to remove the row from.
 * @param primaryKeys	The primary keys of the rows to remove.
 */
void Database::removeRows(QWidget* parent, NormalTable& table, QSet<ValidItemID> primaryKeys)
{
	assert(databaseLoaded);
	
	Database::removeRows_referenceSearch(parent, false, table, primaryKeys);
	
	for (const ValidItemID& primaryKey : qAsConst(primaryKeys)) {
		table.removeRow(parent, {&table.primaryKeyColumn}, {primaryKey});
	}
}

/**
 * For the items with the given primary keys in the given table, performs an exhaustive search for
 * all references to them in other tables and either collects or removes them.
 * 
 * @see WhatIfDeleteResult
 * 
 * @pre A database file is currently open.
 * 
 * @param parent			The parent window.
 * @param searchNotExecute	Whether to collect references (true) or remove them (false).
 * @param table				The table from which a row is to be removed.
 * @param primaryKeys		The primary keys of the rows to be removed.
 * @return					A list of WhatIfDeleteResults which lists the reference count for each affected table. Empty if searchNotExecute is false.
 */
QList<WhatIfDeleteResult> Database::removeRows_referenceSearch(QWidget* parent, bool searchNotExecute, NormalTable& table, QSet<ValidItemID> primaryKeys)
{
	assert(databaseLoaded);
	
	const PrimaryKeyColumn& primaryKeyColumn = table.primaryKeyColumn;
	QList<WhatIfDeleteResult> result = QList<WhatIfDeleteResult>();
	for (Table* candidateTable : tables) {
		if (candidateTable == &table) continue;
		if (candidateTable->getNumberOfRows() == 0) continue;	// No conflicts in empty tables
		
		// Look for references in associative table
		if (candidateTable->isAssociative) {
			AssociativeTable* candidateAssociativeTable = (AssociativeTable*) candidateTable;
			
			const PrimaryForeignKeyColumn* matchingColumn = candidateAssociativeTable->getOwnColumnReferencing(primaryKeyColumn);
			if (!matchingColumn) continue;
			
			// WHAT IF
			if (searchNotExecute) {
				int numAffectedRowIndices = 0;
				for (const ValidItemID& primaryKey : primaryKeys) {
					numAffectedRowIndices += candidateAssociativeTable->getNumberOfMatchingRows(*matchingColumn, primaryKey);
				}
				if (numAffectedRowIndices > 0) {
					const NormalTable* itemTable = candidateAssociativeTable->traverseAssociativeRelation(primaryKeyColumn);
					result.append(WhatIfDeleteResult(candidateAssociativeTable, itemTable, numAffectedRowIndices));
				}
			}
			// EXECUTE
			else {
				for (const ValidItemID& primaryKey : primaryKeys) {
					candidateAssociativeTable->removeMatchingRows(parent, *matchingColumn, primaryKey);
				}
			}
		}
		
		// Look for references in normal table
		else {
			NormalTable* candidateNormalTable = (NormalTable*) candidateTable;
			
			QSet<BufferRowIndex> affectedRowIndices = QSet<BufferRowIndex>();
			QList<QPair<const Column*, QList<BufferRowIndex>>> affectedCells = QList<QPair<const Column*, QList<BufferRowIndex>>>();
			
			for (const Column* otherTableColumn : candidateNormalTable->getColumnList()) {
				if (otherTableColumn->getReferencedForeignColumn() != &primaryKeyColumn) continue;
				
				for (const ValidItemID& primaryKey : primaryKeys) {
					QList<BufferRowIndex> rowIndexList = candidateNormalTable->getMatchingBufferRowIndices(*otherTableColumn, ID_GET(primaryKey));
					QSet<BufferRowIndex> rowIndexSet = QSet<BufferRowIndex>(rowIndexList.constBegin(), rowIndexList.constEnd());
					
					affectedRowIndices.unite(rowIndexSet);
					affectedCells.append({ otherTableColumn, rowIndexList });
				}
			}
			
			// WHAT IF
			if (searchNotExecute) {
				if (!affectedRowIndices.isEmpty()) {
					result.append(WhatIfDeleteResult(candidateNormalTable, candidateNormalTable, affectedRowIndices.size()));
				}
			}
			// EXECUTE
			else {
				for (const auto& [affectedColumn, rowIndices] : affectedCells) {
					NormalTable* affectedTable = (NormalTable*) affectedColumn->table;
					const PrimaryKeyColumn& affectedPrimaryKeyColumn = affectedTable->primaryKeyColumn;
					
					for (const BufferRowIndex& rowIndex : rowIndices) {
						ValidItemID primaryKey = VALID_ITEM_ID(affectedPrimaryKeyColumn.getValueAt(rowIndex));
						// Remove single instance of reference to the key about to be removed
						candidateNormalTable->updateCell(parent, primaryKey, *affectedColumn, ItemID().asQVariant());
					}
				}
			}
		}
		
	}
	return result;
}





/**
 * Creates a new WhatIfDeleteResult.
 * 
 * @param affectedTable			The table that would be affected by the deletion.
 * @param itemTable				The table that contains the item that is to be deleted.
 * @param numAffectedRowIndices	The number of rows in the affected table that would be affected by the deletion.
 */
WhatIfDeleteResult::WhatIfDeleteResult(const Table* affectedTable, const NormalTable* itemTable, int numAffectedRowIndices) :
	affectedTable(affectedTable),
	itemTable(itemTable),
	numAffectedRowIndices(numAffectedRowIndices)
{}
