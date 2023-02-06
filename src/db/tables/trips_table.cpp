#include "trips_table.h"

#include <QString>
#include <QTranslator>



TripsTable::TripsTable() :
		NormalTable(QString("Trips"), QString("trip"), QObject::tr("Trips")),
		//							name					uiName						type		nullable	primaryKey	foreignKey	inTable
		nameColumn		(new Column(QString("name"),		QObject::tr("Name"),		varchar,	true,		false,		nullptr,	this)),
		startDateColumn	(new Column(QString("startDate"),	QObject::tr("Start date"),	date,		true,		false,		nullptr,	this)),
		endDateColumn	(new Column(QString("endDate"),		QObject::tr("End date"),	date,		true,		false,		nullptr,	this)),
		notesColumn		(new Column(QString("notes"),		QObject::tr("Notes"),		varchar,	true,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(startDateColumn);
	addColumn(endDateColumn);
	addColumn(notesColumn);
}



void TripsTable::addRow(Trip* trip)
{
	// TODO
}
