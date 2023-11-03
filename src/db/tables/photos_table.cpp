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



/**
 * Creates a new PhotosTable.
 * 
 * @param foreignAscentIDColumn	The primary key column of the AscentsTable.
 */
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



/**
 * Returns the photos associated with a given ascentID.
 * 
 * @param ascentID	The ascentID to look for.
 * @return			The photos associated with the ascentID.
 */
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



/**
 * Adds one or more rows to the table, using a given ascentID and a list of photos.
 *
 * @param parent	The parent widget.
 * @param ascentID	The ascentID to use for all rows.
 * @param photos	The photos to add.
 */
void PhotosTable::addRows(QWidget* parent, ValidItemID ascentID, const QList<Photo>& photos)
{
	for (int i = 0; i < photos.size(); i++) {
		QList<const Column*> columns = getNonPrimaryKeyColumnList();
		QList<QVariant> data = mapDataToQVariantList(columns, ascentID, i, photos.at(i).filepath, photos.at(i).description);
		
		NormalTable::addRow(parent, columns, data);
	}
}

/**
 * Adds one or more rows to the table, using the data from an ascent object.
 *
 * @param parent	The parent widget.
 * @param ascent	The ascent object from which to get the ascentID and photos.
 */
void PhotosTable::addRows(QWidget* parent, const Ascent* ascent)
{
	return addRows(parent, ascent->ascentID.forceValid(), ascent->photos);
}

/**
 * Updates the table contents for a given ascentID.
 * 
 * First removes all existing rows where the ascentID matches, then adds rows for all photos in the
 * given list.
 *
 * @param parent	The parent widget.
 * @param ascentID	The ascentID for which to update the table contents.
 * @param photos	The photos to add.
 */
void PhotosTable::updateRows(QWidget* parent, ValidItemID ascentID, const QList<Photo>& photos)
{
	// delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascentID);
	// add back all current rows
	addRows(parent, ascentID, photos);
}

/**
 * Updates the table contents for a given ascent.
 *
 * @param parent	The parent widget.
 * @param ascent	The ascent to use for updating the table contents.
 */
void PhotosTable::updateRows(QWidget* parent, const Ascent* ascent)
{
	return updateRows(parent, ascent->ascentID.forceValid(), ascent->photos);
}

/**
 * Updates the filepath of a photo in the table.
 * 
 * @param parent			The parent widget.
 * @param bufferRowIndex	The index of the photo in the table buffer.
 * @param newFilepath		The new filepath to set.
 */
void PhotosTable::updateFilepathAt(QWidget* parent, BufferRowIndex bufferRowIndex, QString newFilepath)
{
	ValidItemID primaryKey = getPrimaryKeyAt(bufferRowIndex);
	updateCellInNormalTable(parent, primaryKey, filepathColumn, newFilepath);
}


/**
 * Translates the data of a photo to a list of QVariants.
 *
 * @param columns		The column list specifying the order of the data.
 * @param ascentID		The ascentID of the photo.
 * @param sortIndex		The sortIndex of the photo.
 * @param filepath		The filepath of the photo.
 * @param description	The description of the photo.
 * @return				The list of QVariants representing the photo.
 */
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



/**
 * Returns the translated string to be displayed to indicate that no photo is selected.
 *
 * @return	The translated string representing absence of a photo.
 */
QString PhotosTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns the translation of "photo" (singular), not capitalized unless the language requires it.
 *
 * @return	The translation of "photo" (singular) for use mid-sentence.
 */
QString PhotosTable::getItemNameSingularLowercase() const
{
	return tr("photo");
}

/**
 * Returns the translation of "photos" (plural), not capitalized unless the language requires it.
 *
 * @return	The translation of "photos" (plural) for use mid-sentence.
 */
QString PhotosTable::getItemNamePluralLowercase() const
{
	return tr("photos");
}
