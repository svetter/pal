#include "db_interface.h"

#include "src/db/db_error.h"
#include "src/db/test_db.h"
#include "src/main/main_window.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>



Database::Database(MainWindow* parent) :
		parent(parent)
{
	tripsTable			= new TripsTable();
	hikersTable			= new HikersTable();
	rangesTable			= new RangesTable();
	countriesTable		= new CountriesTable();
	regionsTable		= new RegionsTable(rangesTable->getPrimaryKeyColumn(), countriesTable->getPrimaryKeyColumn());
	peaksTable			= new PeaksTable(regionsTable->getPrimaryKeyColumn());
	ascentsTable		= new AscentsTable(peaksTable->getPrimaryKeyColumn(), tripsTable->getPrimaryKeyColumn());
	photosTable			= new PhotosTable(ascentsTable->getPrimaryKeyColumn());
	participatedTable	= new ParticipatedTable(ascentsTable->getPrimaryKeyColumn(), hikersTable->getPrimaryKeyColumn());
	
	QSqlError initError = initDB();
	if (initError.type() != QSqlError::NoError) {
		displayError(parent, initError);
	}
	
	ascentsTable->initBuffer(parent);
	countriesTable->initBuffer(parent);
	hikersTable->initBuffer(parent);
	peaksTable->initBuffer(parent);
	rangesTable->initBuffer(parent);
	regionsTable->initBuffer(parent);
	tripsTable->initBuffer(parent);
	photosTable->initBuffer(parent);
	participatedTable->initBuffer(parent);
}

Database::~Database() {
	delete tripsTable;
	delete hikersTable;
	delete rangesTable;
	delete countriesTable;
	delete regionsTable;
	delete peaksTable;
	delete ascentsTable;
	delete photosTable;
	delete participatedTable;
}



Ascent* Database::getAscent(int ascentID) const
{
	return getAscentAt(ascentsTable->getBufferIndexForPrimaryKey(ascentID));
}

Peak* Database::getPeak(int peakID) const
{
	return getPeakAt(peaksTable->getBufferIndexForPrimaryKey(peakID));
}

Trip* Database::getTrip(int tripID) const
{
	return getTripAt(tripsTable->getBufferIndexForPrimaryKey(tripID));
}

Hiker* Database::getHiker(int hikerID) const
{
	return getHikerAt(hikersTable->getBufferIndexForPrimaryKey(hikerID));
}

Region* Database::getRegion(int regionID) const
{
	return getRegionAt(regionsTable->getBufferIndexForPrimaryKey(regionID));
}

Range* Database::getRange(int rangeID) const
{
	return getRangeAt(rangesTable->getBufferIndexForPrimaryKey(rangeID));
}

Country* Database::getCountry(int countryID) const
{
	return getCountryAt(countriesTable->getBufferIndexForPrimaryKey(countryID));
}



Ascent* Database::getAscentAt(int rowIndex) const
{
	const QList<QVariant>* row = ascentsTable->getBufferRow(rowIndex);
	assert(row->size() == ascentsTable->getNumberOfColumns());
	
	int			ascentID			= row->at(ascentsTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		title				= row->at(ascentsTable->titleColumn->getIndex()).toString();
	int			peakID				= row->at(ascentsTable->peakIDColumn->getIndex()).toInt();
	QDate		date				= row->at(ascentsTable->dateColumn->getIndex()).toDate();
	int			perDayIndex			= row->at(ascentsTable->peakOnDayColumn->getIndex()).toInt();
	QTime		time				= row->at(ascentsTable->timeColumn->getIndex()).toTime();
	int			hikeKind			= row->at(ascentsTable->hikeKindColumn->getIndex()).toInt();
	bool		traverse			= row->at(ascentsTable->traverseColumn->getIndex()).toBool();
	int			difficultySystem	= row->at(ascentsTable->difficultySystemColumn->getIndex()).toInt();
	int			difficultyGrade		= row->at(ascentsTable->difficultyGradeColumn->getIndex()).toInt();
	int			tripID				= row->at(ascentsTable->tripIDColumn->getIndex()).toInt();
	QString		description			= row->at(ascentsTable->descriptionColumn->getIndex()).toString();
	
	QSet<int>	hikerIDs			= participatedTable->getMatchingEntries(participatedTable->ascentIDColumn, ascentID);
	QStringList	photos				= photosTable->getPhotosForAscent(ascentID);
	
	return new Ascent(ascentID, title, peakID, date, perDayIndex, time, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, hikerIDs, photos, description);
}

Peak* Database::getPeakAt(int rowIndex) const
{
	const QList<QVariant>* row = peaksTable->getBufferRow(rowIndex);
	assert(row->size() == peaksTable->getNumberOfColumns());
	
	int		peakID		= row->at(peaksTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString	name		= row->at(peaksTable->nameColumn->getIndex()).toString();
	int		height		= row->at(peaksTable->heightColumn->getIndex()).toInt();
	bool	volcano		= row->at(peaksTable->volcanoColumn->getIndex()).toBool();
	int		regionID	= row->at(peaksTable->regionIDColumn->getIndex()).toInt();
	QString	mapsLink	= row->at(peaksTable->mapsLinkColumn->getIndex()).toString();
	QString	earthLink	= row->at(peaksTable->earthLinkColumn->getIndex()).toString();
	QString	wikiLink	= row->at(peaksTable->wikiLinkColumn->getIndex()).toString();
	
	return new Peak(peakID, name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
}

Trip* Database::getTripAt(int rowIndex) const
{
	const QList<QVariant>* row = tripsTable->getBufferRow(rowIndex);
	assert(row->size() == tripsTable->getNumberOfColumns());
	
	int		tripID		= row->at(tripsTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString	name		= row->at(tripsTable->nameColumn->getIndex()).toString();
	QDate	startDate	= row->at(tripsTable->startDateColumn->getIndex()).toDate();
	QDate	endDate		= row->at(tripsTable->endDateColumn->getIndex()).toDate();
	QString	description	= row->at(tripsTable->descriptionColumn->getIndex()).toString();
	
	return new Trip(tripID, name, startDate, endDate, description);
}

Hiker* Database::getHikerAt(int rowIndex) const
{
	const QList<QVariant>* row = hikersTable->getBufferRow(rowIndex);
	assert(row->size() == hikersTable->getNumberOfColumns());
	
	int		hikerID	= row->at(hikersTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString	name	= row->at(hikersTable->nameColumn->getIndex()).toString();
	
	return new Hiker(hikerID, name);
}

Region* Database::getRegionAt(int rowIndex) const
{
	const QList<QVariant>* row = regionsTable->getBufferRow(rowIndex);
	assert(row->size() == regionsTable->getNumberOfColumns());
	
	int		regionID	= row->at(regionsTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString	name		= row->at(regionsTable->nameColumn->getIndex()).toString();
	int		rangeID		= row->at(regionsTable->rangeIDColumn->getIndex()).toInt();
	int		countryID	= row->at(regionsTable->countryIDColumn->getIndex()).toInt();
	
	return new Region(regionID, name, rangeID, countryID);
}

Range* Database::getRangeAt(int rowIndex) const
{
	const QList<QVariant>* row = rangesTable->getBufferRow(rowIndex);
	assert(row->size() == rangesTable->getNumberOfColumns());
	
	int		rangeID		= row->at(rangesTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString	name		= row->at(rangesTable->nameColumn->getIndex()).toString();
	int		continent	= row->at(rangesTable->continentColumn->getIndex()).toInt();
	
	return new Range(rangeID, name, continent);
}

Country* Database::getCountryAt(int rowIndex) const
{
	const QList<QVariant>* row = countriesTable->getBufferRow(rowIndex);
	assert(row->size() == countriesTable->getNumberOfColumns());
	
	int		countryID	= row->at(countriesTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString	name		= row->at(countriesTable->nameColumn->getIndex()).toString();
	
	return new Country(countryID, name);
}



WhatIfResult Database::whatIf_removeRow(NormalTable* table, int primaryKey) const {
	// TODO
	assert(false);
	return WhatIfResult();
}

WhatIfResult Database::whatIf_removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2) const {
	// TODO
	assert(false);
	return WhatIfResult();
}

bool Database::removeRow(NormalTable* table, int primaryKey) {
	// TODO
	assert(false);
	return false;
}

bool Database::removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2) {
	// TODO
	assert(false);
	return false;
}


WhatIfResult Database::whatIf_changeCell(Column* column, int primaryKey) const {	// NormalTables only
	// TODO
	assert(false);
	return WhatIfResult();
}

bool Database::changeCell(Column* column, int primaryKey, QVariant& cell) {	// NormalTables only
	// TODO
	assert(false);
	return false;
}



int Database::getIntFromRecord(QSqlQuery& query, QString& queryString, int entryInd) const
{
	assert(entryInd >= 0);
	QVariant variantValue = query.value(entryInd);
	if (!variantValue.isValid())
		displayError(parent, "Received invalid QVariant from query", queryString);
	bool conversionOk;
	int intValue = variantValue.toInt(&conversionOk);
	if (!conversionOk)
		displayError(parent, "Conversion to int failed for result from query", queryString);
	return intValue;
}

QString Database::getStringFromRecord(QSqlQuery& query, QString& queryString, int entryInd) const
{
	QVariant variantValue = query.value(entryInd);
	if (!variantValue.isValid())
		displayError(parent, "Received invalid QVariant from query", queryString);
	QString stringValue = variantValue.toString();
	return stringValue;
}
