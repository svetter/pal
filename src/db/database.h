#ifndef DATABASE_H
#define DATABASE_H

#include "src/data/ascent.h"
#include "src/data/peak.h"
#include "src/data/trip.h"
#include "src/data/hiker.h"
#include "src/data/region.h"
#include "src/data/range.h"
#include "src/data/country.h"
#include "src/db/project_settings.h"
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
class CompositeAscentsTable;
struct WhatIfDeleteResult;



class Database {
	bool databaseLoaded;
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
	
	ProjectSettings*	projectSettings;
	
	Database();
	~Database();
	
	void reset();
	void createNew(QWidget* parent, const QString& filepath);
	void openExisting(QWidget* parent, const QString& filepath);
	bool saveAs(QWidget* parent, const QString& filepath);
	QString getCurrentFilepath() const;
	
	void setStatusBar(QStatusBar* mainWindowStatusBar);
	
	QList<Table*> getTableList() const;
	
	Ascent*		getAscent	(ValidItemID ascentID)	const;
	Peak*		getPeak		(ValidItemID peakID)	const;
	Trip*		getTrip		(ValidItemID tripID)	const;
	Hiker*		getHiker	(ValidItemID hikerID)	const;
	Region*		getRegion	(ValidItemID regionID)	const;
	Range*		getRange	(ValidItemID rangeID)	const;
	Country*	getCountry	(ValidItemID countryID)	const;
	
	Ascent*		getAscentAt		(int rowIndex) const;
	Peak*		getPeakAt		(int rowIndex) const;
	Trip*		getTripAt		(int rowIndex) const;
	Hiker*		getHikerAt		(int rowIndex) const;
	Region*		getRegionAt		(int rowIndex) const;
	Range*		getRangeAt		(int rowIndex) const;
	Country*	getCountryAt	(int rowIndex) const;
	
	QList<WhatIfDeleteResult> whatIf_removeRow(NormalTable* table, ValidItemID primaryKey);
	void removeRow(QWidget* parent, NormalTable* table, ValidItemID primaryKey);
private:
	QList<WhatIfDeleteResult> removeRow_referenceSearch(QWidget* parent, bool searchNotExecute, NormalTable* table, ValidItemID primaryKey);
	
	void populateBuffers(QWidget* parent, bool expectEmpty = false);
	
	void setStatusBarMessage(QString content) const;
	
	int getIntFromRecord(QWidget* parent, QSqlQuery& query, QString& queryString, int entryInd) const;
	QString getStringFromRecord(QWidget* parent, QSqlQuery& query, QString& queryString, int entryInd) const;
	
public:
	void insertTestData(QWidget* parent);
};



struct WhatIfDeleteResult {
	const Table*		affectedTable;
	const NormalTable*	itemTable;
	int					numAffectedRowIndices;
	
	WhatIfDeleteResult(const Table* affectedTable, const NormalTable* itemTable, int numAffectedRowIndices);
};



#endif // DATABASE_H
