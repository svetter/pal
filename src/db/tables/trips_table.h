#ifndef TRIPS_TABLE_H
#define TRIPS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/trip.h"

#include <QWidget>



class TripsTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* const nameColumn;
	Column* const startDateColumn;
	Column* const endDateColumn;
	Column* const descriptionColumn;
	
	TripsTable();
	
	int addRow(QWidget* parent, Trip* trip);
	void updateRow(QWidget* parent, ValidItemID tripID, const Trip* trip);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Trip* trip) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // TRIPS_TABLE_H
