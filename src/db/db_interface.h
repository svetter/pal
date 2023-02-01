#ifndef DB_INTERFACE_H
#define DB_INTERFACE_H

#include "db_model.h"



struct WhatIfResult {
	QList<Table*>	affectedTables;
	int				numAffectedRows;
};


class RowContents;
class CellContent;


class Database {
public:
	const NormalTable * const		ascentsTable;
	const NormalTable * const		peaksTable;
	const NormalTable * const		tripsTable;
	const NormalTable * const		hikersTable;
	const NormalTable * const		regionsTable;
	const NormalTable * const		rangesTable;
	const NormalTable * const		countriesTable;
	const NormalTable * const		photosTable;
	const AssociativeTable * const	participatesTable;
	
	Database();		// Create tables and populate them with columns
	~Database();
	
	Ascent*		getAscent(int ascentID);
	Peak*		getPeak(int peakID);
	Trip*		getTrip(int tripID);
	Hiker*		getHiker(int hikerID);
	Region*		getRegion(int regionID);
	Range*		getRange(int rangeID);
	Country*	getCountry(int countryID);
	Photo*		getPhoto(int photoID);
	
	bool addRow(Table * table, RowContents row);
	
	WhatIfResult whatIf_removeRow(NormalTable * table, int primaryKey) const;
	WhatIfResult whatIf_removeRow(AssociativeTable * table, int primaryForeignKey1, int primaryForeignKey2) const;
	bool removeRow(NormalTable * table, int primaryKey);
	bool removeRow(AssociativeTable * table, int primaryForeignKey1, int primaryForeignKey2);
	
	WhatIfResult whatIf_changeCell(Column * column, int primaryKey) const;	// NormalTables only
	bool changeCell(Column * column, int primaryKey, CellContent cell);		// NormalTables only
};



#endif // DB_INTERFACE_H
