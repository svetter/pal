#ifndef TRIPS_TABLE_H
#define TRIPS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/trip.h"

#include <QWidget>



class TripsTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* nameColumn;
	Column* startDateColumn;
	Column* endDateColumn;
	Column* descriptionColumn;
	
	TripsTable();
	
	int addRow(QWidget* parent, const Trip* trip);
};



#endif // TRIPS_TABLE_H
