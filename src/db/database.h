#ifndef DATABASE_H
#define DATABASE_H

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

#include <QStatusBar>
#include <QSqlQuery>
#include <QSqlError>

class MainWindow;



class Database {
	QList<Table*> tables;
	
	QStatusBar* mainWindowStatusBar;
	
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
	
	void setStatusBar(QStatusBar* mainWindowStatusBar);
	
	QList<Table*> getTableList() const;
	
	Ascent*		getAscent	(int ascentID)	const;
	Peak*		getPeak		(int peakID)	const;
	Trip*		getTrip		(int tripID)	const;
	Hiker*		getHiker	(int hikerID)	const;
	Region*		getRegion	(int regionID)	const;
	Range*		getRange	(int rangeID)	const;
	Country*	getCountry	(int countryID)	const;
	
	Ascent*		getAscentAt		(int rowIndex) const;
	Peak*		getPeakAt		(int rowIndex) const;
	Trip*		getTripAt		(int rowIndex) const;
	Hiker*		getHikerAt		(int rowIndex) const;
	Region*		getRegionAt		(int rowIndex) const;
	Range*		getRangeAt		(int rowIndex) const;
	Country*	getCountryAt	(int rowIndex) const;
	
	QList<WhatIfDeleteResult> whatIf_removeRow(NormalTable* table, int primaryKey) const;
	bool removeRow(NormalTable* table, int primaryKey);
	bool removeRow(AssociativeTable* table, int primaryForeignKey1, int primaryForeignKey2);
	
private:
	void setStatusBarMessage(QString content) const;
	
	int getIntFromRecord(QWidget* parent, QSqlQuery& query, QString& queryString, int entryInd) const;
	QString getStringFromRecord(QWidget* parent, QSqlQuery& query, QString& queryString, int entryInd) const;
};



#endif // DATABASE_H
