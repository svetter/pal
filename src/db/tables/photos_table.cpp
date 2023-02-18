#include "photos_table.h"

#include <QString>
#include <QMap>
#include <QTranslator>



PhotosTable::PhotosTable(const Column* foreignAscentIDColumn) :
		NormalTable(QString("Photos"), QString("photo"), QString(), QString()),
		//										name			uiName				type		nullable	primaryKey	foreignKey				inTable
		ascentIDColumn	(new const Column(QString("ascentID"),	QString(),			integer,	true,		false,		foreignAscentIDColumn,	this)),
		sortIndexColumn	(new const Column(QString("sortIndex"),	tr("Sort index"),	integer,	true,		false,		nullptr,				this)),
		filepathColumn	(new const Column(QString("filepath"),	tr("File path"),	varchar,	true,		false,		nullptr,				this))
{
	addColumn(ascentIDColumn);
	addColumn(sortIndexColumn);
	addColumn(filepathColumn);
}



QStringList PhotosTable::getPhotosForAscent(int ascentID) const
{
	QMap<int, QString> filtered = QMap<int, QString>();
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(ascentIDColumn->getIndex()) == ascentID) {
			int photoIndex = (*iter)->at(sortIndexColumn->getIndex()).toInt();
			QString filepath = (*iter)->at(filepathColumn->getIndex()).toString();
			filtered.insert(photoIndex, filepath);
		}
	}
	QStringList sortedList = filtered.values();
	return sortedList;
}



void PhotosTable::addRows(QWidget* parent, const Ascent* ascent)
{
	assert(ascent->ascentID > -1);
	int sortIndex = 0;
	for (auto iter = ascent->photos.constBegin(); iter != ascent->photos.constEnd(); iter++) {
		addRow(parent, ascent->ascentID, sortIndex, *iter);
		sortIndex++;
	}
}

void PhotosTable::addRow(QWidget* parent, int ascentID, int sortIndex, const QString& filepath)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == ascentIDColumn)	{ data.append(ascentID);	continue; }
		if (*iter == sortIndexColumn)	{ data.append(sortIndex);	continue; }
		if (*iter == filepathColumn)	{ data.append(filepath);	continue; }
		assert(false);
	}
	NormalTable::addRow(parent, data);
}
