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
	assert(false);
	assert(ascentID > 0);
	QString queryString = QString(
			"SELECT " + ascentsTable->getColumnListString() +
			"\nFROM " + ascentsTable->getName() +
			"\nWHERE " + ascentsTable->getPrimaryKeyColumn()->getName() + "=" + QString::number(ascentID)
	);
	
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
	if (!query.isActive())
		displayError(parent, "query.isActive() returned false", queryString);
	if (!query.isSelect())
		displayError(parent, "query.isSelect() returned false", queryString);
	if (!query.next()) {
		displayError(parent, "Couldn't read record from SQL query", queryString);
	}
	QVariant variantValue = query.value(0);
	if (query.next())
		displayError(parent, "More than one record returned for query", queryString);
	if (!variantValue.isValid())
		displayError(parent, "Received invalid QVariant from query", queryString);
}

Ascent* Database::getAscentAt(int rowIndex) const
{
	const QList<QVariant>* row = ascentsTable->getBufferRow(rowIndex);
	
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
	
	QList<int>	hikerIDs			= participatedTable->getMatchingEntries(participatedTable->ascentIDColumn, ascentID);
	QStringList	photos				= photosTable->getPhotosForAscent(ascentID);
	
	return new Ascent(-1, title, peakID, date, perDayIndex, time, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, hikerIDs, photos, description);
}

Peak* Database::getPeak(int peakID) const {
	
}

Trip* Database::getTrip(int tripID) const {
	
}

Hiker* Database::getHiker(int hikerID) const {
	assert(hikerID > 0);
	QString queryString = QString(
			"SELECT " + hikersTable->getColumnListString() +
			"\nFROM " + hikersTable->getName() +
			"\nWHERE " + hikersTable->getPrimaryKeyColumn()->getName() + "=" + QString::number(hikerID)
	);
	
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	if (!query.exec(queryString))
		displayError(parent, query.lastError(), queryString);
	if (!query.next()) {
		displayError(parent, "Couldn't read record from SQL query", queryString);
	}
	int readHikerID		= getIntFromRecord(query, queryString, hikersTable->getColumnIndex(hikersTable->getPrimaryKeyColumn()));
	QString readName	= getStringFromRecord(query, queryString, hikersTable->getColumnIndex(hikersTable->nameColumn));
	if (query.next())
		displayError(parent, "More than one record returned for query", queryString);
	return new Hiker(readHikerID, readName);
}

Region* Database::getRegion(int regionID) const {
	
}

Range* Database::getRange(int rangeID) const {
	
}

Country* Database::getCountry(int countryID) const {
	
}



int Database::addRow(NormalTable* table, QList<QVariant>& row) {
	QString questionMarks = "";
	for (int i = 0; i < table->getNumberOfNonPrimaryKeyColumns(); i++) {
		questionMarks = questionMarks + ((i == 0) ? "?" : ", ?");
	}
	QString queryString = QString(
			"INSERT INTO " + table->getName() + "(" + table->getNonPrimaryKeyColumnListString() + ")" +
			"\nVALUES(" + questionMarks + ")"
	);
	QSqlQuery query = QSqlQuery();
	if (!query.prepare(queryString))
		displayError(parent, query.lastError(), queryString);
	for (auto iter = row.constBegin(); iter != row.constEnd(); iter++) {
		query.addBindValue(*iter);
	}
	if (!query.exec())
		displayError(parent, query.lastError(), queryString);
	int index = query.lastInsertId().toInt();
	assert(index > 0);
	return index;
}


WhatIfResult Database::whatIf_removeRow(NormalTable* table, int primaryKey) const {
	
}

WhatIfResult Database::whatIf_removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2) const {
	
}

bool Database::removeRow(NormalTable* table, int primaryKey) {
	
}

bool Database::removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2) {
	
}


WhatIfResult Database::whatIf_changeCell(Column* column, int primaryKey) const {	// NormalTables only
	
}

bool Database::changeCell(Column* column, int primaryKey, QVariant& cell) {	// NormalTables only
	
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
