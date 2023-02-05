#include "db_interface.h"

#include "src/db/db_tables.h"
#include "src/db/test_db.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>



Database::Database(QMainWindow* parent) :
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
		displayError(initError);
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



Ascent* Database::getAscent(int ascentID) {
	
}

Peak* Database::getPeak(int peakID) {
	
}

Trip* Database::getTrip(int tripID) {
	
}

Hiker* Database::getHiker(int hikerID) {
	
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



bool Database::addRow(Table* table, QList<QVariant>& row) {
	
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



void Database::displayError(QString error)
{
	QMessageBox::critical(parent, "Database error", error);
	exit(1);
}

void Database::displayError(QString error, QString& queryString)
{
	return displayError(error + "\n\nQuery:\n" + queryString);
}

void Database::displayError(QSqlError error, QString& queryString)
{
	return displayError(error.text(), queryString);
}

void Database::displayError(QSqlError error)
{
	return displayError(error.text());
}
