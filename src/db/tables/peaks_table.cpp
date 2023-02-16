#include "peaks_table.h"

#include <QString>
#include <QTranslator>



PeaksTable::PeaksTable(Column* foreignRegionIDColumn) :
		NormalTable(QString("Peaks"), QString("peak"), tr("Peaks"), tr("None")),
		//							name					uiName						type		nullable	primaryKey	foreignKey				inTable
		nameColumn		(new Column(QString("name"),		tr("Name"),					varchar,	false,		false,		nullptr,				this)),
		heightColumn	(new Column(QString("height"),		tr("Height"),				integer,	true,		false,		nullptr,				this)),
		volcanoColumn	(new Column(QString("volcano"),		tr("Volcano"),				bit,		false,		false,		nullptr,				this)),
		regionIDColumn	(new Column(QString("regionID"),	QString(),					integer,	true,		false,		foreignRegionIDColumn,	this)),
		mapsLinkColumn	(new Column(QString("mapsLink"),	tr("Google Maps link"),		varchar,	true,		false,		nullptr,				this)),
		earthLinkColumn	(new Column(QString("earthLink"),	tr("Google Earth link"),	varchar,	true,		false,		nullptr,				this)),
		wikiLinkColumn	(new Column(QString("wikiLink"),	tr("Wikipedia link"),		varchar,	true,		false,		nullptr,				this))
{
	addColumn(nameColumn);
	addColumn(heightColumn);
	addColumn(volcanoColumn);
	addColumn(regionIDColumn);
	addColumn(mapsLinkColumn);
	addColumn(earthLinkColumn);
	addColumn(wikiLinkColumn);
}



int PeaksTable::addRow(QWidget* parent, const Peak* peak)
{
	assert(peak->peakID == -1);
	QList<Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == nameColumn)		{ data.append(peak->name);		continue; }
		if (*iter == heightColumn)		{ data.append(peak->height);	continue; }
		if (*iter == volcanoColumn)		{ data.append(peak->volcano);	continue; }
		if (*iter == regionIDColumn)	{ data.append(peak->regionID);	continue; }
		if (*iter == mapsLinkColumn)	{ data.append(peak->mapsLink);	continue; }
		if (*iter == earthLinkColumn)	{ data.append(peak->earthLink);	continue; }
		if (*iter == wikiLinkColumn)	{ data.append(peak->wikiLink);	continue; }
		assert(false);
	}
	int newPeakIndex = NormalTable::addRow(parent, data);
	return newPeakIndex;
}
