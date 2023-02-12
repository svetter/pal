#include "db_interface.h"

#include "src/db/db_error.h"
#include "src/db/test_db.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>



Database::Database(QWidget* parent) :
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



QList<Hiker*>* Database::getAllHikers()
{
	QString queryString = QString(
			"SELECT " + hikersTable->getColumnListString() +
			"\nFROM " + hikersTable->getName()
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	QList<Hiker*>* list = new QList<Hiker*>();
	
	if (!query.exec(queryString))
		displayError(query.lastError(), queryString);
	
	while (query.next()) {
		int readHikerID		= getIntFromRecord(query, queryString, hikersTable->getColumnIndex(hikersTable->getPrimaryKeyColumn()));
		QString readName	= getStringFromRecord(query, queryString, hikersTable->getColumnIndex(hikersTable->nameColumn));
		
		*list += new Hiker(readHikerID, readName);
	}
	
	if (list->empty())
		displayError("Couldn't read record from SQL query", queryString);
	return list;
}



Ascent* Database::getAscent(int ascentID) {
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

Peak* Database::getPeak(int peakID) {
	
}

Trip* Database::getTrip(int tripID) {
	
}

Hiker* Database::getHiker(int hikerID) {
	assert(hikerID > 0);
	QString queryString = QString(
			"SELECT " + hikersTable->getColumnListString() +
			"\nFROM " + hikersTable->getName() +
			"\nWHERE " + ascentsTable->getPrimaryKeyColumn()->getName() + "=" + QString::number(hikerID)
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

Region* Database::getRegion(int regionID) {
	
}

Range* Database::getRange(int rangeID) {
	
}

Country* Database::getCountry(int countryID) {
	
}



int Database::getNumberOfEntries(NormalTable* table)
{
	QString queryString = QString(
			"SELECT COUNT(" + table->getPrimaryKeyColumn()->getName() + ")" +
			"\nFROM " + table->getName()
	);
	QSqlQuery query = QSqlQuery();
	query.setForwardOnly(true);
	if (!query.exec(queryString))
		displayError(query.lastError(), queryString);
	if (!query.next()) {
		displayError("Couldn't read record from SQL query \"" + queryString + "\"", queryString);
	}
	QVariant variantValue = query.value(0);
	if (!variantValue.isValid())
		displayError("Received invalid QVariant from query", queryString);
	if (query.next())
		displayError("More than one record returned for query", queryString);
	bool intConversionOk;
	int intValue = variantValue.toInt(&intConversionOk);
	if (!intConversionOk)
		displayError("Conversion to int failed for result from query", queryString);
	return intValue;
}



int Database::addRow(NormalTable* table, QList<QVariant>& row) {
	QString queryString = QString(
			"INSERT INTO " + table->getName() + "(" + table->getNonPrimaryKeyColumnListString() + ")" +
			"\nVALUES(" + repeat("?", table->getNumberOfNonPrimaryKeyColumns()) + ")"
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



int Database::getIntFromRecord(QSqlQuery& query, QString& queryString, int entryInd)
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

QString Database::getStringFromRecord(QSqlQuery& query, QString& queryString, int entryInd)
{
	QVariant variantValue = query.value(entryInd);
	if (!variantValue.isValid())
		displayError(parent, "Received invalid QVariant from query", queryString);
	QString stringValue = variantValue.toString();
	return stringValue;
}



QString Database::repeat(QString string, int times)
{
	assert(times >= 0);
	QString result = "";
	for (int i = 0; i < times; i++) {
		result = result + ((i == 0) ? string : (", " + string));
	}
	return result;
}
