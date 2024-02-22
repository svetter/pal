/*
 * Copyright 2023-2024 Simon Vetter
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
PhotosTable::PhotosTable(PrimaryKeyColumn& foreignAscentIDColumn) :
	NormalTable(QString("Photos"), tr("Photos"), "photoID", tr("Photo ID")),
	//												name			uiName				type		nullable	foreignColumn
	ascentIDColumn		(ForeignKeyColumn	(this,	"ascentID",		tr("Ascent ID"),				true,		foreignAscentIDColumn)),
	sortIndexColumn		(ValueColumn		(this,	"sortIndex",	tr("Sort index"),	Integer,	true)),
	filepathColumn		(ValueColumn		(this,	"filepath",		tr("File path"),	String,		false)),
	descriptionColumn	(ValueColumn		(this,	"description",	tr("Description"),	String,		true))
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
	QList<BufferRowIndex> bufferRowIndices = getMatchingBufferRowIndices(ascentIDColumn, ID_GET(ascentID));
	
	QMap<int, Photo> photosMap = QMap<int, Photo>();
	for (BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		int sortIndex		= sortIndexColumn	.getValueAt(bufferRowIndex).toInt();
		QString filepath	= filepathColumn	.getValueAt(bufferRowIndex).toString();
		QString description	= descriptionColumn	.getValueAt(bufferRowIndex).toString();
		
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
		const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, ascentID, i, photos.at(i).filepath, photos.at(i).description);
		
		NormalTable::addRow(parent, columnDataPairs);
	}
}

/**
 * Adds one or more rows to the table, using the data from an ascent object.
 *
 * @param parent	The parent widget.
 * @param ascent	The ascent object from which to get the ascentID and photos.
 */
void PhotosTable::addRows(QWidget* parent, const Ascent& ascent)
{
	return addRows(parent, FORCE_VALID(ascent.ascentID), ascent.photos);
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
void PhotosTable::updateRows(QWidget* parent, const Ascent& ascent)
{
	return updateRows(parent, FORCE_VALID(ascent.ascentID), ascent.photos);
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
 * Translates the data of a photo to a list of column-data pairs.
 *
 * @param columns		The column list specifying the order of the data.
 * @param ascentID		The ascentID of the photo.
 * @param sortIndex		The sortIndex of the photo.
 * @param filepath		The filepath of the photo.
 * @param description	The description of the photo.
 * @return				A list of column-data pairs representing the photo.
 */
const QList<ColumnDataPair> PhotosTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, ValidItemID ascentID, int sortIndex, const QString& filepath, const QString& description) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &ascentIDColumn)		{ data = ascentID.asQVariant();	}
		else if (column == &sortIndexColumn)	{ data = sortIndex;				}
		else if (column == &filepathColumn)		{ data = filepath;				}
		else if (column == &descriptionColumn)	{ data = description;			}
		else assert(false);
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
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
