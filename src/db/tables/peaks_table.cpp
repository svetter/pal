#include "peaks_table.h"

#include <QString>
#include <QTranslator>



PeaksTable::PeaksTable(Column* foreignRegionIDColumn) :
		NormalTable(QString("Peaks"), QString("peak"), QObject::tr("Peaks")),
		//							name					uiName								type		nullable	primaryKey	foreignKey				inTable
		nameColumn		(new Column(QString("name"),		QObject::tr("Name"),				varchar,	false,		false,		nullptr,				this)),
		heightColumn	(new Column(QString("height"),		QObject::tr("Height"),				integer,	true,		false,		nullptr,				this)),
		volcanoColumn	(new Column(QString("volcano"),		QObject::tr("Volcano"),				bit,		false,		false,		nullptr,				this)),
		regionIDColumn	(new Column(QString("regionID"),	QString(),							integer,	true,		false,		foreignRegionIDColumn,	this)),
		mapsLinkColumn	(new Column(QString("mapsLink"),	QObject::tr("Google Maps link"),	varchar,	true,		false,		nullptr,				this)),
		earthLinkColumn	(new Column(QString("earthLink"),	QObject::tr("Google Earth link"),	varchar,	true,		false,		nullptr,				this)),
		wikiLinkColumn	(new Column(QString("wikiLink"),	QObject::tr("Wikipedia link"),		varchar,	true,		false,		nullptr,				this))
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
	// TODO
}
