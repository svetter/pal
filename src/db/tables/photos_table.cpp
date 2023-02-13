#include "photos_table.h"

#include <QString>
#include <QMap>
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



QList<QString> PhotosTable::getPhotosForAscent(int ascentID) const
{
	QMap<int, QString> filtered = QMap<int, QString>();
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(ascentIDColumn->getIndex()) == ascentID) {
			int photoIndex = (*iter)->at(indexColumn->getIndex()).toInt();
			QString filepath = (*iter)->at(filepathColumn->getIndex()).toString();
			filtered.insert(photoIndex, filepath);
		}
	}
	QList<QString> sortedList = filtered.values();
	return sortedList;
}



int PhotosTable::addRow()
{
	// TODO #97
	return -1;
}
