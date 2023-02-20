#include "trips_table.h"

#include <QString>
#include <QTranslator>



TripsTable::TripsTable() :
		NormalTable(QString("Trips"), tr("Trips"), "tripID"),
		//											name				uiName				type		nullable	primaryKey	foreignKey	inTable
		nameColumn			(new const Column(QString("name"),			tr("Name"),			varchar,	true,		false,		nullptr,	this)),
		startDateColumn		(new const Column(QString("startDate"),		tr("Start date"),	date,		true,		false,		nullptr,	this)),
		endDateColumn		(new const Column(QString("endDate"),		tr("End date"),		date,		true,		false,		nullptr,	this)),
		descriptionColumn	(new const Column(QString("description"),	tr("Description"),	varchar,	true,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(startDateColumn);
	addColumn(endDateColumn);
	addColumn(descriptionColumn);
}



int TripsTable::addRow(QWidget* parent, Trip* trip)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, trip);
	
	int newTripIndex = NormalTable::addRow(parent, data);
	trip->tripID = buffer->at(newTripIndex)->at(getPrimaryKeyColumn()->getIndex()).toInt();
	return newTripIndex;
}

void TripsTable::updateRow(QWidget* parent, ValidItemID tripID, const Trip* trip)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, trip);
	
	NormalTable::updateRow(parent, tripID, columns, data);
}


QList<QVariant> TripsTable::mapDataToQVariantList(QList<const Column*>& columns, const Trip* trip) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)			{ data.append(trip->name);			continue; }
		if (column == startDateColumn)		{ data.append(trip->startDate);		continue; }
		if (column == endDateColumn)		{ data.append(trip->endDate);		continue; }
		if (column == descriptionColumn)	{ data.append(trip->description);	continue; }
		assert(false);
	}
	return data;
}



QString TripsTable::getNoneString() const
{
	return tr("None");
}

QString TripsTable::getItemNameSingularLowercase() const
{
	return tr("trip");
}

QString TripsTable::getItemNamePluralLowercase() const
{
	return tr("trips");
}
