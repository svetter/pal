#ifndef TRIPS_TABLE_H
#define TRIPS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/trip.h"



class TripsTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* nameColumn;
	Column* startDateColumn;
	Column* endDateColumn;
	Column* descriptionColumn;
	
	TripsTable();
	
	int addRow(Trip* trip);
};



#endif // TRIPS_TABLE_H
