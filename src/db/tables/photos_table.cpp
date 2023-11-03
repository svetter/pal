/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file photos_table.cpp
 * 
 * This file defines the PhotosTable class.
 */

#include "photos_table.h"

#include <QString>
#include <QMap>
#include <QTranslator>



PhotosTable::PhotosTable(Column* foreignAscentIDColumn) :
		NormalTable(QString("Photos"), QString(), "photoID"),
		//								name			uiName				type		nullable	primaryKey	foreignKey				inTable
		ascentIDColumn		(new Column("ascentID",		QString(),			ID,			true,		false,		foreignAscentIDColumn,	this)),
		sortIndexColumn		(new Column("sortIndex",	tr("Sort index"),	Integer,	true,		false,		nullptr,				this)),
		filepathColumn		(new Column("filepath",		tr("File path"),	String,		false,		false,		nullptr,				this)),
		descriptionColumn	(new Column("description",	tr("Description"),	String,		true,		false,		nullptr,				this))
{
	addColumn(primaryKeyColumn);
	addColumn(ascentIDColumn);
	addColumn(sortIndexColumn);
	addColumn(filepathColumn);
	addColumn(descriptionColumn);
}



QList<Photo> PhotosTable::getPhotosForAscent(ValidItemID ascentID) const
{
	QList<BufferRowIndex> bufferRowIndices = getMatchingBufferRowIndices(ascentIDColumn, ascentID.get());
	
	QMap<int, Photo> photosMap = QMap<int, Photo>();
	for (BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		int sortIndex		= sortIndexColumn	->getValueAt(bufferRowIndex).toInt();
		QString filepath	= filepathColumn	->getValueAt(bufferRowIndex).toString();
		QString description	= descriptionColumn	->getValueAt(bufferRowIndex).toString();
		
		Photo newPhoto = Photo(ItemID(), ascentID, sortIndex, filepath, description);
		photosMap.insert(sortIndex, newPhoto);
	}
	QList<Photo> sortedList = photosMap.values();
	return sortedList;
}



void PhotosTable::addRows(QWidget* parent, ValidItemID ascentID, const QList<Photo>& photos)
{
	for (int i = 0; i < photos.size(); i++) {
		QList<const Column*> columns = getNonPrimaryKeyColumnList();
		QList<QVariant> data = mapDataToQVariantList(columns, ascentID, i, photos.at(i).filepath, photos.at(i).description);
		
		NormalTable::addRow(parent, columns, data);
	}
}

void PhotosTable::addRows(QWidget* parent, const Ascent* ascent)
{
	return addRows(parent, ascent->ascentID.forceValid(), ascent->photos);
}

void PhotosTable::updateRows(QWidget* parent, ValidItemID ascentID, const QList<Photo>& photos)
{
	// delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascentID);
	// add back all current rows
	addRows(parent, ascentID, photos);
}

void PhotosTable::updateRows(QWidget* parent, const Ascent* ascent)
{
	return updateRows(parent, ascent->ascentID.forceValid(), ascent->photos);
}

void PhotosTable::updateFilepathAt(QWidget* parent, BufferRowIndex bufferRowIndex, QString newFilepath)
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
