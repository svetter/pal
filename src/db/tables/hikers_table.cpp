#include "hikers_table.h"

#include <QString>
#include <QTranslator>



HikersTable::HikersTable() :
		NormalTable(QString("Hikers"), QString("hiker"), QObject::tr("Hikers")),
		//						name				uiName					type		nullable	primaryKey	foreignKey	inTable
		nameColumn	(new Column(QString("name"),	QObject::tr("Name"),	varchar,	false,		false,		nullptr,	this))
{
	addColumn(nameColumn);
}



int HikersTable::addRow(Hiker* hiker)
{
	// TODO #97
}
