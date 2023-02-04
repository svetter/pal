#include "db_interface.h"

#include "src/db/db_tables.h"



Database::Database() {
	tripsTable			= new TripsTable();
	hikersTable			= new HikersTable();
	rangesTable			= new RangesTable();
	countriesTable		= new CountriesTable();
	regionsTable		= new RegionsTable(rangesTable->getPrimaryKeyColumn(), countriesTable->getPrimaryKeyColumn());
	peaksTable			= new PeaksTable(regionsTable->getPrimaryKeyColumn());
	ascentsTable		= new AscentsTable(peaksTable->getPrimaryKeyColumn(), tripsTable->getPrimaryKeyColumn());
	photosTable			= new PhotosTable(ascentsTable->getPrimaryKeyColumn());
	participatedTable	= new ParticipatedTable(ascentsTable->getPrimaryKeyColumn(), hikersTable->getPrimaryKeyColumn());
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
