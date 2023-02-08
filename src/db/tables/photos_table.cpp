#include "photos_table.h"

#include <QString>
#include <QTranslator>



PhotosTable::PhotosTable(Column* foreignAscentIDColumn) :
		NormalTable(QString("Photos"), QString("photo"), QString()),
		//							name					uiName						type		nullable	primaryKey	foreignKey				inTable
		ascentIDColumn	(new Column(QString("ascentID"),	QString(),					integer,	true,		false,		foreignAscentIDColumn,	this)),
		indexColumn		(new Column(QString("photoIndex"),	QObject::tr("Index"),		integer,	true,		false,		nullptr,				this)),
		filepathColumn	(new Column(QString("filepath"),	QObject::tr("File path"),	varchar,	true,		false,		nullptr,				this))
{
	addColumn(ascentIDColumn);
	addColumn(indexColumn);
	addColumn(filepathColumn);
}



int PhotosTable::addRows(Ascent* ascent)
{
	// TODO
}
