#include "ranges_table.h"

#include <QString>
#include <QTranslator>



RangesTable::RangesTable() :
		NormalTable(QString("Ranges"), QString("range"), tr("Mountain ranges"), tr("None")),
		//							name					uiName				type		nullable	primaryKey	foreignKey	inTable
		nameColumn		(new Column(QString("name"),		tr("Name"),			varchar,	false,		false,		nullptr,	this)),
		continentColumn	(new Column(QString("continent"),	tr("Continent"),	integer,	true,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(continentColumn);
}



int RangesTable::addRow(Range* range)
{
	// TODO #97
	return -1;
}
