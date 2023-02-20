#include "photos_table.h"

#include <QString>
#include <QMap>
#include <QTranslator>



PhotosTable::PhotosTable(const Column* foreignAscentIDColumn) :
		NormalTable(QString("Photos"), QString(), "photoID"),
		//										name			uiName				type		nullable	primaryKey	foreignKey				inTable
		ascentIDColumn	(new const Column(QString("ascentID"),	QString(),			integer,	true,		false,		foreignAscentIDColumn,	this)),
		sortIndexColumn	(new const Column(QString("sortIndex"),	tr("Sort index"),	integer,	true,		false,		nullptr,				this)),
		filepathColumn	(new const Column(QString("filepath"),	tr("File path"),	varchar,	true,		false,		nullptr,				this))
{
	addColumn(ascentIDColumn);
	addColumn(sortIndexColumn);
	addColumn(filepathColumn);
}



QStringList PhotosTable::getPhotosForAscent(ValidItemID ascentID) const
{
	QMap<int, QString> filtered = QMap<int, QString>();
	for (auto iter = buffer->constBegin(); iter != buffer->constEnd(); iter++) {
		if ((*iter)->at(ascentIDColumn->getIndex()) == ascentID.get()) {
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
	for (int i = 0; i < ascent->photos.size(); i++) {
		QList<const Column*> columns = getNonPrimaryKeyColumnList();
		QList<QVariant> data = mapDataToQVariantList(columns, ascent->ascentID.forceValid(), i, ascent->photos.at(i));
		
		NormalTable::addRow(parent, data);
	}
}

void PhotosTable::updateRows(QWidget* parent, const Ascent* ascent)
{
	// delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascent->ascentID.forceValid());
	// add back all current rows
	addRows(parent, ascent);
}


QList<QVariant> PhotosTable::mapDataToQVariantList(QList<const Column*>& columns, ValidItemID ascentID, int sortIndex, const QString& filepath) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == ascentIDColumn)	{ data.append(ascentID.asQVariant());	continue; }
		if (column == sortIndexColumn)	{ data.append(sortIndex);				continue; }
		if (column == filepathColumn)	{ data.append(filepath);				continue; }
		assert(false);
	}
	return data;
}



QString PhotosTable::getNoneString() const
{
	return tr("None");
}

QString PhotosTable::getItemNameSingularLowercase() const
{
	return tr("photo");
}

QString PhotosTable::getItemNamePluralLowercase() const
{
	return tr("photos");
}
