#include "peaks_table.h"

#include <QString>
#include <QTranslator>



PeaksTable::PeaksTable(const Column* foreignRegionIDColumn) :
		NormalTable(QString("Peaks"), tr("Peaks"), "peakID"),
		//										name			uiName						type		nullable	primaryKey	foreignKey				inTable
		nameColumn		(new const Column(QString("name"),		tr("Name"),					varchar,	false,		false,		nullptr,				this)),
		heightColumn	(new const Column(QString("height"),	tr("Height"),				integer,	true,		false,		nullptr,				this)),
		volcanoColumn	(new const Column(QString("volcano"),	tr("Volcano"),				bit,		false,		false,		nullptr,				this)),
		regionIDColumn	(new const Column(QString("regionID"),	QString(),					integer,	true,		false,		foreignRegionIDColumn,	this)),
		mapsLinkColumn	(new const Column(QString("mapsLink"),	tr("Google Maps link"),		varchar,	true,		false,		nullptr,				this)),
		earthLinkColumn	(new const Column(QString("earthLink"),	tr("Google Earth link"),	varchar,	true,		false,		nullptr,				this)),
		wikiLinkColumn	(new const Column(QString("wikiLink"),	tr("Wikipedia link"),		varchar,	true,		false,		nullptr,				this))
{
	addColumn(nameColumn);
	addColumn(heightColumn);
	addColumn(volcanoColumn);
	addColumn(regionIDColumn);
	addColumn(mapsLinkColumn);
	addColumn(earthLinkColumn);
	addColumn(wikiLinkColumn);
}



int PeaksTable::addRow(QWidget* parent, Peak* peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, peak);
	
	int newPeakIndex = NormalTable::addRow(parent, data);
	peak->peakID = buffer->at(newPeakIndex)->at(getPrimaryKeyColumn()->getIndex()).toInt();
	return newPeakIndex;
}

void PeaksTable::updateRow(QWidget* parent, ValidItemID hikerID, const Peak* peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, peak);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> PeaksTable::mapDataToQVariantList(QList<const Column*>& columns, const Peak* peak) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)		{ data.append(peak->name);					continue; }
		if (column == heightColumn)		{ data.append(peak->height);				continue; }
		if (column == volcanoColumn)	{ data.append(peak->volcano);				continue; }
		if (column == regionIDColumn)	{ data.append(peak->regionID.asQVariant());	continue; }
		if (column == mapsLinkColumn)	{ data.append(peak->mapsLink);				continue; }
		if (column == earthLinkColumn)	{ data.append(peak->earthLink);				continue; }
		if (column == wikiLinkColumn)	{ data.append(peak->wikiLink);				continue; }
		assert(false);
	}
	return data;
}



QString PeaksTable::getNoneString() const
{
	return tr("None");
}

QString PeaksTable::getItemNameSingularLowercase() const
{
	return tr("peak");
}

QString PeaksTable::getItemNamePluralLowercase() const
{
	return tr("peaks");
}
