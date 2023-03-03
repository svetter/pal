#include "photos_table.h"

#include <QString>
#include <QMap>
#include <QTranslator>



PhotosTable::PhotosTable(const Column* foreignAscentIDColumn) :
		NormalTable(QString("Photos"), QString(), "photoID"),
		//											name				uiName					type		nullable	primaryKey	foreignKey				inTable
		ascentIDColumn		(new const Column(QString("ascentID"),		QString(),				integer,	true,		false,		foreignAscentIDColumn,	this)),
		sortIndexColumn		(new const Column(QString("sortIndex"),		tr("Sort index"),		integer,	true,		false,		nullptr,				this)),
		filepathColumn		(new const Column(QString("filepath"),		tr("File path"),		varchar,	false,		false,		nullptr,				this)),
		descriptionColumn	(new const Column(QString("description"),	tr("Description"),		varchar,	true,		false,		nullptr,				this))
{
	addColumn(primaryKeyColumn);
	addColumn(ascentIDColumn);
	addColumn(sortIndexColumn);
	addColumn(filepathColumn);
	addColumn(descriptionColumn);
}



QList<Photo> PhotosTable::getPhotosForAscent(ValidItemID ascentID) const
{
	QList<int> bufferRowIndices = getMatchingBufferRowIndices(ascentIDColumn, ascentID.get());
	
	QMap<int, Photo> photosMap = QMap<int, Photo>();
	for (int bufferRowIndex : bufferRowIndices) {
		const QList<QVariant>* bufferRow = getBufferRow(bufferRowIndex);
		
		int sortIndex		= bufferRow->at(sortIndexColumn		->getIndex()).toInt();
		QString filepath	= bufferRow->at(filepathColumn		->getIndex()).toString();
		QString description	= bufferRow->at(descriptionColumn	->getIndex()).toString();
		
		Photo newPhoto = Photo(ItemID(), ascentID, sortIndex, filepath, description);
		photosMap.insert(sortIndex, newPhoto);
	}
	QList<Photo> sortedList = photosMap.values();
	return sortedList;
}



void PhotosTable::addRows(QWidget* parent, const Ascent* ascent)
{
	const QList<Photo>& photos = ascent->photos;
	for (int i = 0; i < photos.size(); i++) {
		QList<const Column*> columns = getNonPrimaryKeyColumnList();
		QList<QVariant> data = mapDataToQVariantList(columns, ascent->ascentID.forceValid(), i, ascent->photos.at(i).filepath, ascent->photos.at(i).description);
		
		NormalTable::addRow(parent, columns, data);
	}
}

void PhotosTable::updateRows(QWidget* parent, const Ascent* ascent)
{
	// delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascent->ascentID.forceValid());
	// add back all current rows
	addRows(parent, ascent);
}

void PhotosTable::updateFilepathAt(QWidget* parent, int bufferRowIndex, QString newFilepath)
{
	ValidItemID primaryKey = getPrimaryKeyAt(bufferRowIndex);
	updateCellInNormalTable(parent, primaryKey, filepathColumn, newFilepath);
}


QList<QVariant> PhotosTable::mapDataToQVariantList(QList<const Column*>& columns, ValidItemID ascentID, int sortIndex, const QString& filepath, const QString& description) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == ascentIDColumn)		{ data.append(ascentID.asQVariant());	continue; }
		if (column == sortIndexColumn)		{ data.append(sortIndex);				continue; }
		if (column == filepathColumn)		{ data.append(filepath);				continue; }
		if (column == descriptionColumn)	{ data.append(description);				continue; }
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
