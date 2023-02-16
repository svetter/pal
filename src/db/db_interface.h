#ifndef DB_INTERFACE_H
#define DB_INTERFACE_H

#include "associative_table.h"
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

#include <QSqlQuery>
#include <QSqlError>

class MainWindow;



class Database {
	MainWindow* parent;
	
public:
	AscentsTable*		ascentsTable;
	PeaksTable*			peaksTable;
	TripsTable*			tripsTable;
	HikersTable*		hikersTable;
	RegionsTable*		regionsTable;
	RangesTable*		rangesTable;
	CountriesTable*		countriesTable;
	PhotosTable*		photosTable;
	ParticipatedTable*	participatedTable;
	
	Database(MainWindow* parent);
	~Database();
	
	Ascent*		getAscent(int ascentID) const;
	Peak*		getPeak(int peakID) const;
	Trip*		getTrip(int tripID) const;
	Hiker*		getHiker(int hikerID) const;
	Region*		getRegion(int regionID) const;
	Range*		getRange(int rangeID) const;
	Country*	getCountry(int countryID) const;
	
	Ascent*		getAscentAt(int rowIndex) const;
	
	int addRow(NormalTable* table, QList<QVariant>& row);
//	bool addRow(AssociativeTable* table, QList<QVariant>& row);
	
	WhatIfResult whatIf_removeRow(NormalTable* table, int primaryKey) const;
	WhatIfResult whatIf_removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2) const;
	bool removeRow(NormalTable* table, int primaryKey);
	bool removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2);
	
	WhatIfResult whatIf_changeCell(Column* column, int primaryKey) const;	// NormalTables only
	bool changeCell(Column* column, int primaryKey, QVariant& cell);		// NormalTables only
	
private:
	int getIntFromRecord(QSqlQuery& query, QString& queryString, int entryInd) const;
	QString getStringFromRecord(QSqlQuery& query, QString& queryString, int entryInd) const;
};



#endif // DB_INTERFACE_H
