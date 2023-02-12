#include "photos_table.h"

#include <QString>
#include <QTranslator>



PhotosTable::PhotosTable(Column* foreignAscentIDColumn) :
		NormalTable(QString("Photos"), QString("photo"), QString(), QString()),
		//							name					uiName				type		nullable	primaryKey	foreignKey				inTable
		ascentIDColumn	(new Column(QString("ascentID"),	QString(),			integer,	true,		false,		foreignAscentIDColumn,	this)),
		indexColumn		(new Column(QString("sortIndex"),	tr("Index"),		integer,	true,		false,		nullptr,				this)),
		filepathColumn	(new Column(QString("filepath"),	tr("File path"),	varchar,	true,		false,		nullptr,				this))
{
	addColumn(ascentIDColumn);
	addColumn(indexColumn);
	addColumn(filepathColumn);
}



int PhotosTable::addRow()
{
	// TODO #97
}
