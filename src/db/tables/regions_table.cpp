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
	QList<QVariant> data = mapDataToQVariantList(region);
	
	int newRegionIndex = NormalTable::addRow(parent, data);
	return newRegionIndex;
}

void RegionsTable::updateRow(QWidget* parent, ValidItemID hikerID, const Region* region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(region);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> RegionsTable::mapDataToQVariantList(const Region* region) const
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)		{ data.append(region->name);					continue; }
		if (column == rangeIDColumn)	{ data.append(region->rangeID.asQVariant());	continue; }
		if (column == countryIDColumn)	{ data.append(region->countryID.asQVariant());	continue; }
		assert(false);
	}
	return data;
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
