#include "ranges_table.h"

#include <QString>
#include <QTranslator>



RangesTable::RangesTable() :
		NormalTable(QString("Ranges"), QString("range"), QObject::tr("Mountain ranges")),
		//							name					uiName						type		nullable	primaryKey	foreignKey	inTable
		nameColumn		(new Column(QString("name"),		QObject::tr("Name"),		varchar,	false,		false,		nullptr,	this)),
		continentColumn	(new Column(QString("continent"),	QObject::tr("Continent"),	integer,	true,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(continentColumn);
}



int RangesTable::addRow(Range* range)
{
	// TODO
}
