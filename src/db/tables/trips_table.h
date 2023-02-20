#ifndef TRIPS_TABLE_H
#define TRIPS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/trip.h"

#include <QWidget>



class TripsTable : public NormalTable {
	Q_OBJECT
	
public:
	const Column* nameColumn;
	const Column* startDateColumn;
	const Column* endDateColumn;
	const Column* descriptionColumn;
	
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
