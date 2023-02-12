#include "countries_table.h"

#include <QString>
#include <QTranslator>



CountriesTable::CountriesTable() :
		NormalTable(QString("Countries"), QString("country"), tr("Countries"), tr("None")),
		//						name				uiName		type		nullable	primaryKey	foreignKey	inTable
		nameColumn	(new Column(QString("name"),	tr("Name"),	varchar,	false,		false,		nullptr,	this))
{
	addColumn(nameColumn);
}



int CountriesTable::addRow(Country* country)
{
	// TODO #97
	return -1;
}