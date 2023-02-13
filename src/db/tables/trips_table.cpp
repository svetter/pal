#include "trips_table.h"

#include <QString>
#include <QTranslator>



TripsTable::TripsTable() :
		NormalTable(QString("Trips"), QString("trip"), tr("Trips"), tr("None")),
		//								name					uiName				type		nullable	primaryKey	foreignKey	inTable
		nameColumn			(new Column(QString("name"),		tr("Name"),			varchar,	true,		false,		nullptr,	this)),
		startDateColumn		(new Column(QString("startDate"),	tr("Start date"),	date,		true,		false,		nullptr,	this)),
		endDateColumn		(new Column(QString("endDate"),		tr("End date"),		date,		true,		false,		nullptr,	this)),
		descriptionColumn	(new Column(QString("description"),	tr("Description"),	varchar,	true,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(startDateColumn);
	addColumn(endDateColumn);
	addColumn(descriptionColumn);
}



int TripsTable::addRow(QWidget* parent, const Trip* trip)
{
	QList<Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == nameColumn)		{ data.append(trip->name);			continue; }
		if (*iter == startDateColumn)	{ data.append(trip->startDate);		continue; }
		if (*iter == endDateColumn)		{ data.append(trip->endDate);		continue; }
		if (*iter == descriptionColumn)	{ data.append(trip->description);	continue; }
		assert(false);
	}
	int newTripIndex = NormalTable::addRow(parent, data);
	return newTripIndex;
}
