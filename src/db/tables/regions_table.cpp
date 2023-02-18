#include "regions_table.h"

#include <QString>
#include <QTranslator>



RegionsTable::RegionsTable(const Column* foreignRangeIDColumn, const Column* foreignCountryIDColumn) :
		NormalTable(QString("Regions"), tr("Regions"), "regionID"),
		//										name			uiName		type		nullable	primaryKey	foreignKey				inTable
		nameColumn		(new const Column(QString("name"),		tr("Name"),	varchar,	false,		false,		nullptr,				this)),
		rangeIDColumn	(new const Column(QString("rangeID"),	QString(),	integer,	true,		false,		foreignRangeIDColumn,	this)),
		countryIDColumn	(new const Column(QString("countryID"),	QString(),	integer,	true,		false,		foreignCountryIDColumn,	this))
{
	addColumn(nameColumn);
	addColumn(rangeIDColumn);
	addColumn(countryIDColumn);
}



int RegionsTable::addRow(QWidget* parent, const Region* region)
{
	assert(region->regionID == -1);
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == nameColumn)		{ data.append(region->name);		continue; }
		if (*iter == rangeIDColumn)		{ data.append(region->rangeID);		continue; }
		if (*iter == countryIDColumn)	{ data.append(region->countryID);	continue; }
		assert(false);
	}
	int newRegionIndex = NormalTable::addRow(parent, data);
	return newRegionIndex;
}



QString RegionsTable::getNoneString() const
{
	return tr("None");
}

QString RegionsTable::getItemNameSingularLowercase() const
{
	return tr("region");
}

QString RegionsTable::getItemNamePluralLowercase() const
{
	return tr("regions");
}
