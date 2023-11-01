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

#include "database.h"

#include "src/db/db_error.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QFile>



Database::Database() :
		databaseLoaded(false),
		tables(QList<Table*>()),
		mainWindowStatusBar(nullptr),
		projectSettings(new ProjectSettings())
{
	tripsTable			= new TripsTable();
	hikersTable			= new HikersTable();
	rangesTable			= new RangesTable();
	countriesTable		= new CountriesTable();
	regionsTable		= new RegionsTable(rangesTable->primaryKeyColumn, countriesTable->primaryKeyColumn);
	peaksTable			= new PeaksTable(regionsTable->primaryKeyColumn);
	ascentsTable		= new AscentsTable(peaksTable->primaryKeyColumn, tripsTable->primaryKeyColumn);
	photosTable			= new PhotosTable(ascentsTable->primaryKeyColumn);
	participatedTable	= new ParticipatedTable(ascentsTable->primaryKeyColumn, hikersTable->primaryKeyColumn);
	
	tables.append(tripsTable);
	tables.append(hikersTable);
	tables.append(rangesTable);
	tables.append(countriesTable);
	tables.append(regionsTable);
	tables.append(peaksTable);
	tables.append(ascentsTable);
	tables.append(photosTable);
	tables.append(participatedTable);
	
	
	QSqlDatabase::addDatabase("QSQLITE");
}

Database::~Database() {
	qDeleteAll(getTableList());
	delete projectSettings;
}



void Database::reset()
{
	for (Table* table : tables) {
		table->resetBuffer();
	}
	projectSettings->resetBuffer();
	
	QSqlDatabase::database().close();
	
	databaseLoaded = false;
}

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
	if (!sql.open())
		displayError(parent, sql.lastError());
	databaseLoaded = true;
	
	qDebug() << "Creating tables in SQL";
	for (Table* table : tables) {
		table->createTableInSql(parent);
	}
	projectSettings->createTableInSql(parent);
	projectSettings->initBuffer(parent, true);
	projectSettings->insertDefaults(parent);
	
	// All tables still empty of course, but this doubles as a table format check
	populateBuffers(parent, true);
}

void Database::openExisting(QWidget* parent, const QString& filepath)
{
	assert(!databaseLoaded);
	qDebug() << "Opening database file" << filepath;
	
	// Set filename
	QSqlDatabase sql = QSqlDatabase::database();
	sql.setDatabaseName(filepath);
	
	// Open connection
	if (!sql.open())
		displayError(parent, sql.lastError());
	databaseLoaded = true;
	
	populateBuffers(parent);
	projectSettings->initBuffer(parent);
}

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


QString Database::getCurrentFilepath() const
{
	assert(databaseLoaded);
	QSqlDatabase sql = QSqlDatabase::database();
	QString filepath = sql.databaseName();
	return filepath;
}


void Database::populateBuffers(QWidget* parent, bool expectEmpty)
{
	assert(databaseLoaded);
	
	for (Table* table : tables) {
		assert(table->getNumberOfRows() == 0);
		table->initBuffer(parent, expectEmpty);
	}
}



QList<Table*> Database::getTableList() const
{
	return QList<Table*>(tables);
}



Ascent* Database::getAscent(ValidItemID ascentID) const
{
	assert(databaseLoaded);
	return getAscentAt(ascentsTable->getBufferIndexForPrimaryKey(ascentID));
}

Peak* Database::getPeak(ValidItemID peakID) const
{
	assert(databaseLoaded);
	return getPeakAt(peaksTable->getBufferIndexForPrimaryKey(peakID));
}

Trip* Database::getTrip(ValidItemID tripID) const
{
	assert(databaseLoaded);
	return getTripAt(tripsTable->getBufferIndexForPrimaryKey(tripID));
}

Hiker* Database::getHiker(ValidItemID hikerID) const
{
	assert(databaseLoaded);
	return getHikerAt(hikersTable->getBufferIndexForPrimaryKey(hikerID));
}

Region* Database::getRegion(ValidItemID regionID) const
{
	assert(databaseLoaded);
	return getRegionAt(regionsTable->getBufferIndexForPrimaryKey(regionID));
}

Range* Database::getRange(ValidItemID rangeID) const
{
	assert(databaseLoaded);
	return getRangeAt(rangesTable->getBufferIndexForPrimaryKey(rangeID));
}

Country* Database::getCountry(ValidItemID countryID) const
{
	assert(databaseLoaded);
	return getCountryAt(countriesTable->getBufferIndexForPrimaryKey(countryID));
}



Ascent* Database::getAscentAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = ascentsTable->getBufferRow(rowIndex);
	assert(row->size() == ascentsTable->getNumberOfColumns());
	
	ValidItemID	ascentID			= row->at(ascentsTable->primaryKeyColumn->getIndex());
	QString		title				= row->at(ascentsTable->titleColumn->getIndex()).toString();
	ItemID		peakID				= row->at(ascentsTable->peakIDColumn->getIndex());
	QDate		date				= row->at(ascentsTable->dateColumn->getIndex()).toDate();
	int			perDayIndex			= row->at(ascentsTable->peakOnDayColumn->getIndex()).toInt();
	QTime		time				= row->at(ascentsTable->timeColumn->getIndex()).toTime();
	int			elevationGain		= row->at(ascentsTable->elevationGainColumn->getIndex()).toInt();
	int			hikeKind			= row->at(ascentsTable->hikeKindColumn->getIndex()).toInt();
	bool		traverse			= row->at(ascentsTable->traverseColumn->getIndex()).toBool();
	int			difficultySystem	= row->at(ascentsTable->difficultySystemColumn->getIndex()).toInt();
	int			difficultyGrade		= row->at(ascentsTable->difficultyGradeColumn->getIndex()).toInt();
	ItemID		tripID				= row->at(ascentsTable->tripIDColumn->getIndex());
	QString		description			= row->at(ascentsTable->descriptionColumn->getIndex()).toString();
	
	QSet<ValidItemID>	hikerIDs	= participatedTable->getMatchingEntries(participatedTable->ascentIDColumn, ascentID);
	QList<Photo>		photos		= photosTable->getPhotosForAscent(ascentID);
	
	return new Ascent(ascentID, title, peakID, date, perDayIndex, time, elevationGain, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, hikerIDs, photos, description);
}

Peak* Database::getPeakAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = peaksTable->getBufferRow(rowIndex);
	assert(row->size() == peaksTable->getNumberOfColumns());
	
	ValidItemID	peakID		= row->at(peaksTable->primaryKeyColumn->getIndex());
	QString		name		= row->at(peaksTable->nameColumn->getIndex()).toString();
	int			height		= row->at(peaksTable->heightColumn->getIndex()).toInt();
	bool		volcano		= row->at(peaksTable->volcanoColumn->getIndex()).toBool();
	int			regionID	= row->at(peaksTable->regionIDColumn->getIndex()).toInt();
	QString		mapsLink	= row->at(peaksTable->mapsLinkColumn->getIndex()).toString();
	QString		earthLink	= row->at(peaksTable->earthLinkColumn->getIndex()).toString();
	QString		wikiLink	= row->at(peaksTable->wikiLinkColumn->getIndex()).toString();
	
	return new Peak(peakID, name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
}

Trip* Database::getTripAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = tripsTable->getBufferRow(rowIndex);
	assert(row->size() == tripsTable->getNumberOfColumns());
	
	ValidItemID	tripID		= row->at(tripsTable->primaryKeyColumn->getIndex());
	QString		name		= row->at(tripsTable->nameColumn->getIndex()).toString();
	QDate		startDate	= row->at(tripsTable->startDateColumn->getIndex()).toDate();
	QDate		endDate		= row->at(tripsTable->endDateColumn->getIndex()).toDate();
	QString		description	= row->at(tripsTable->descriptionColumn->getIndex()).toString();
	
	return new Trip(tripID, name, startDate, endDate, description);
}

Hiker* Database::getHikerAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = hikersTable->getBufferRow(rowIndex);
	assert(row->size() == hikersTable->getNumberOfColumns());
	
	ValidItemID	hikerID	= row->at(hikersTable->primaryKeyColumn->getIndex());
	QString		name	= row->at(hikersTable->nameColumn->getIndex()).toString();
	
	return new Hiker(hikerID, name);
}

Region* Database::getRegionAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = regionsTable->getBufferRow(rowIndex);
	assert(row->size() == regionsTable->getNumberOfColumns());
	
	ValidItemID	regionID	= row->at(regionsTable->primaryKeyColumn->getIndex());
	QString		name		= row->at(regionsTable->nameColumn->getIndex()).toString();
	int			rangeID		= row->at(regionsTable->rangeIDColumn->getIndex()).toInt();
	int			countryID	= row->at(regionsTable->countryIDColumn->getIndex()).toInt();
	
	return new Region(regionID, name, rangeID, countryID);
}

Range* Database::getRangeAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = rangesTable->getBufferRow(rowIndex);
	assert(row->size() == rangesTable->getNumberOfColumns());
	
	ValidItemID	rangeID		= row->at(rangesTable->primaryKeyColumn->getIndex());
	QString		name		= row->at(rangesTable->nameColumn->getIndex()).toString();
	int			continent	= row->at(rangesTable->continentColumn->getIndex()).toInt();
	
	return new Range(rangeID, name, continent);
}

Country* Database::getCountryAt(BufferRowIndex rowIndex) const
{
	assert(databaseLoaded);
	
	const QList<QVariant>* row = countriesTable->getBufferRow(rowIndex);
	assert(row->size() == countriesTable->getNumberOfColumns());
	
	ValidItemID	countryID	= row->at(countriesTable->primaryKeyColumn->getIndex());
	QString		name		= row->at(countriesTable->nameColumn->getIndex()).toString();
	
	return new Country(countryID, name);
}



QList<WhatIfDeleteResult> Database::whatIf_removeRow(NormalTable* table, ValidItemID primaryKey)
{
	assert(databaseLoaded);
	
	return Database::removeRow_referenceSearch(nullptr, true, table, primaryKey);
}

void Database::removeRow(QWidget* parent, NormalTable* table, ValidItemID primaryKey)
{
	assert(databaseLoaded);
	
	Database::removeRow_referenceSearch(parent, false, table, primaryKey);
	
	table->removeRow(parent, { table->primaryKeyColumn }, { primaryKey });
}

QList<WhatIfDeleteResult> Database::removeRow_referenceSearch(QWidget* parent, bool searchNotExecute, NormalTable* table, ValidItemID primaryKey)
{
	assert(databaseLoaded);
	
	const Column* const primaryKeyColumn = table->primaryKeyColumn;
	QList<WhatIfDeleteResult> result = QList<WhatIfDeleteResult>();
	for (Table* candidateTable : tables) {
		if (candidateTable == table) continue;
		if (candidateTable->getNumberOfRows() == 0) continue;	// No conflicts in empty tables
		
		// Look for references in associative table
		if (candidateTable->isAssociative) {
			AssociativeTable* candidateAssociativeTable = (AssociativeTable*) candidateTable;
			
			const Column* matchingColumn = candidateAssociativeTable->getOwnColumnReferencing(primaryKeyColumn);
			if (!matchingColumn) continue;
			
			// WHAT IF
			if (searchNotExecute) {
				int numAffectedRowIndices = candidateAssociativeTable->getNumberOfMatchingRows(matchingColumn, primaryKey);
				if (numAffectedRowIndices > 0) {
					const NormalTable* itemTable = candidateAssociativeTable->traverseAssociativeRelation(primaryKeyColumn);
					result.append(WhatIfDeleteResult(candidateAssociativeTable, itemTable, numAffectedRowIndices));
				}
			}
			// EXECUTE
			else {
				candidateAssociativeTable->removeMatchingRows(parent, matchingColumn, primaryKey);
			}
		}
		
		// Look for references in normal table
		else {
			NormalTable* candidateNormalTable = (NormalTable*) candidateTable;
			
			QSet<BufferRowIndex> affectedRowIndices = QSet<BufferRowIndex>();
			QList<QPair<const Column*, QList<BufferRowIndex>>> affectedCells = QList<QPair<const Column*, QList<BufferRowIndex>>>();
			
			for (const Column* otherTableColumn : candidateNormalTable->getColumnList()) {
				if (otherTableColumn->getReferencedForeignColumn() != primaryKeyColumn) continue;
				
				QList<BufferRowIndex> rowIndexList = candidateNormalTable->getMatchingBufferRowIndices(otherTableColumn, primaryKey.get());
				QSet<BufferRowIndex> rowIndexSet = QSet<BufferRowIndex>(rowIndexList.constBegin(), rowIndexList.constEnd());
				
				affectedRowIndices.unite(rowIndexSet);
				affectedCells.append({ otherTableColumn, rowIndexList });
			}
			
			// WHAT IF
			if (searchNotExecute) {
				if (!affectedRowIndices.isEmpty()) {
					result.append(WhatIfDeleteResult(candidateNormalTable, candidateNormalTable, affectedRowIndices.size()));
				}
			}
			// EXECUTE
			else {
				for (const QPair<const Column*, QList<BufferRowIndex>>& pair : affectedCells) {
					const Column* affectedColumn = pair.first;
					QList<BufferRowIndex> rowIndices = pair.second;
					NormalTable* affectedTable = (NormalTable*) affectedColumn->table;
					const Column* affectedPrimaryKeyColumn = affectedTable->primaryKeyColumn;
					
					for (const BufferRowIndex& rowIndex : rowIndices) {
						ValidItemID primaryKey = affectedPrimaryKeyColumn->getValueAt(rowIndex);
						// Remove single instance of reference to the key about to be removed
						candidateNormalTable->updateCell(parent, primaryKey, affectedColumn, ItemID().asQVariant());
					}
				}
			}
		}
		
	}
	return result;
}





WhatIfDeleteResult::WhatIfDeleteResult(const Table* affectedTable, const NormalTable* itemTable, int numAffectedRowIndices) :
		affectedTable(affectedTable),
		itemTable(itemTable),
		numAffectedRowIndices(numAffectedRowIndices)
{}
