#include "regions_table.h"

#include <QString>
#include <QTranslator>



RegionsTable::RegionsTable(Column* foreignRangeIDColumn, Column* foreignCountryIDColumn) :
		NormalTable(QString("Regions"), tr("Regions"), "regionID"),
		//							name			uiName		type		nullable	primaryKey	foreignKey				inTable
		nameColumn		(new Column("name",			tr("Name"),	varchar,	false,		false,		nullptr,				this)),
		rangeIDColumn	(new Column("rangeID",		QString(),	integer,	true,		false,		foreignRangeIDColumn,	this)),
		countryIDColumn	(new Column("countryID",	QString(),	integer,	true,		false,		foreignCountryIDColumn,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
	addColumn(rangeIDColumn);
	addColumn(countryIDColumn);
}



int RegionsTable::addRow(QWidget* parent, Region* region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, region);
	
	int newRegionIndex = NormalTable::addRow(parent, columns, data);
	region->regionID = getPrimaryKeyAt(newRegionIndex);
	return newRegionIndex;
}

void RegionsTable::updateRow(QWidget* parent, ValidItemID hikerID, const Region* region)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, region);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> RegionsTable::mapDataToQVariantList(QList<const Column*>& columns, const Region* region) const
{
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
