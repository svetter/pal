#ifndef DB_INTERFACE_H
#define DB_INTERFACE_H

#include "db_model.h"
#include "src/data/ascent.h"
#include "src/data/peak.h"
#include "src/data/trip.h"
#include "src/data/hiker.h"
#include "src/data/region.h"
#include "src/data/range.h"
#include "src/data/country.h"



class Database {
public:
	NormalTable*		ascentsTable;
	NormalTable*		peaksTable;
	NormalTable*		tripsTable;
	NormalTable*		hikersTable;
	NormalTable*		regionsTable;
	NormalTable*		rangesTable;
	NormalTable*		countriesTable;
	NormalTable*		photosTable;
	AssociativeTable*	participatedTable;
	
	Database();		// Create tables and populate them with columns
	~Database();
	
	Ascent*		getAscent(int ascentID);
	Peak*		getPeak(int peakID);
	Trip*		getTrip(int tripID);
	Hiker*		getHiker(int hikerID);
	Region*		getRegion(int regionID);
	Range*		getRange(int rangeID);
	Country*	getCountry(int countryID);
	
	bool addRow(Table* table, QList<QVariant>& row);
	
	WhatIfResult whatIf_removeRow(NormalTable* table, int primaryKey) const;
	WhatIfResult whatIf_removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2) const;
	bool removeRow(NormalTable* table, int primaryKey);
	bool removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2);
	
	WhatIfResult whatIf_changeCell(Column* column, int primaryKey) const;	// NormalTables only
	bool changeCell(Column* column, int primaryKey, QVariant& cell);		// NormalTables only
};



#endif // DB_INTERFACE_H
