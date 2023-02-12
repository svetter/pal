#include "regions_table.h"

#include <QString>
#include <QTranslator>



RegionsTable::RegionsTable(Column* foreignRangeIDColumn, Column* foreignCountryIDColumn) :
		NormalTable(QString("Regions"), QString("region"), tr("Regions")),
		//							name					uiName		type		nullable	primaryKey	foreignKey				inTable
		nameColumn		(new Column(QString("name"),		tr("Name"),	varchar,	false,		false,		nullptr,				this)),
		rangeIDColumn	(new Column(QString("rangeID"),		QString(),	integer,	true,		false,		foreignRangeIDColumn,	this)),
		countryIDColumn	(new Column(QString("countryID"),	QString(),	integer,	true,		false,		foreignCountryIDColumn,	this))
{
	addColumn(nameColumn);
	addColumn(rangeIDColumn);
	addColumn(countryIDColumn);
}



int RegionsTable::addRow(Region* region)
{
	// TODO #97
}
