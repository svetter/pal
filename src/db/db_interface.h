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
#include "src/db/db_tables.h"

#include <QMainWindow>
#include <QSqlQuery>
#include <QSqlError>



class Database {
	QMainWindow* parent;
	
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
	
	Database(QMainWindow* parent);
	~Database();
	
	QList<Hiker*>*	getAllHikers();
	
	Ascent*		getAscent(int ascentID);
	Peak*		getPeak(int peakID);
	Trip*		getTrip(int tripID);
	Hiker*		getHiker(int hikerID);
	Region*		getRegion(int regionID);
	Range*		getRange(int rangeID);
	Country*	getCountry(int countryID);
	
	int getNumberOfEntries(NormalTable* table);
	
	int addRow(NormalTable* table, QList<QVariant>& row);
//	bool addRow(AssociativeTable* table, QList<QVariant>& row);
	
	WhatIfResult whatIf_removeRow(NormalTable* table, int primaryKey) const;
	WhatIfResult whatIf_removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2) const;
	bool removeRow(NormalTable* table, int primaryKey);
	bool removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2);
	
	WhatIfResult whatIf_changeCell(Column* column, int primaryKey) const;	// NormalTables only
	bool changeCell(Column* column, int primaryKey, QVariant& cell);		// NormalTables only
	
private:
	int getIntFromRecord(QSqlQuery& query, QString& queryString, int entryInd);
	QString getStringFromRecord(QSqlQuery& query, QString& queryString, int entryInd);
	
	QString repeat(QString string, int times);
	
	void displayError(QString error);
	void displayError(QString error, QString& queryString);
	void displayError(QSqlError error);
	void displayError(QSqlError error, QString& queryString);
};



#endif // DB_INTERFACE_H
