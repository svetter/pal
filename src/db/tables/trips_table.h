#ifndef TRIPS_TABLE_H
#define TRIPS_TABLE_H

#include "src/db/db_model.h"
#include "src/data/trip.h"



class TripsTable : public NormalTable {
public:
	Column* nameColumn;
	Column* startDateColumn;
	Column* endDateColumn;
	Column* notesColumn;
	
	TripsTable();
	
	int addRow(Trip* trip);
};



#endif // TRIPS_TABLE_H
