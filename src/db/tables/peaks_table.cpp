#include "peaks_table.h"

#include <QString>
#include <QTranslator>



PeaksTable::PeaksTable(Column* foreignRegionIDColumn) :
		NormalTable(QString("Peaks"), QString("peak"), tr("Peaks")),
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



int PeaksTable::addRow(Peak* peak)
{
	// TODO #97
}
