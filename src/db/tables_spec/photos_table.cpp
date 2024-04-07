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
 * @param db					The database to which the table belongs.
 * @param foreignAscentIDColumn	The primary key column of the AscentsTable.
 */
PhotosTable::PhotosTable(Database& db, PrimaryKeyColumn& foreignAscentIDColumn) :
	NormalTable(db, QString("Photos"), tr("Photos"), "photoID", tr("Photo ID")),
	//												name			uiName				type		nullable	foreignColumn
	ascentIDColumn		(ForeignKeyColumn	(*this,	"ascentID",		tr("Ascent ID"),				true,		foreignAscentIDColumn)),
	sortIndexColumn		(ValueColumn		(*this,	"sortIndex",	tr("Sort index"),	Integer,	true)),
	filepathColumn		(ValueColumn		(*this,	"filepath",		tr("File path"),	String,		false)),
	descriptionColumn	(ValueColumn		(*this,	"description",	tr("Description"),	String,		true))
{
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
void PhotosTable::addRows(QWidget& parent, ValidItemID ascentID, const QList<Photo>& photos)
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
void PhotosTable::addRows(QWidget& parent, const Ascent& ascent)
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
void PhotosTable::updateRows(QWidget& parent, ValidItemID ascentID, const QList<Photo>& photos)
{
	// Delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascentID);
	// Add back all current rows
	addRows(parent, ascentID, photos);
}

/**
 * Updates the table contents for a given ascent.
 *
 * @param parent	The parent widget.
 * @param ascent	The ascent to use for updating the table contents.
 */
void PhotosTable::updateRows(QWidget& parent, const Ascent& ascent)
{
	return updateRows(parent, FORCE_VALID(ascent.ascentID), ascent.photos);
}

/**
 * Updates the table contents for a given set of ascentIDs.
 * 
 * First removes all existing rows where the ascentID matches, then adds rows for all photos in the
 * given list, for each ascentID.
 *
 * @param parent		The parent widget.
 * @param ascentIDs	The ascentIDs for which to update the table contents.
 * @param photos		The photos to add.
 */
void PhotosTable::updateRows(QWidget& parent, const QSet<ValidItemID>& ascentIDs, const QList<Photo>& photos)
{
	// Delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascentIDs);
	// Add back all current rows
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	for (const ValidItemID& ascentID : ascentIDs) {
		for (int i = 0; i < photos.size(); i++) {
			const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, ascentID, i, photos.at(i).filepath, photos.at(i).description);
			
			NormalTable::addRow(parent, columnDataPairs);
		}
	}
}

/**
 * Updates the filepath of a photo in the table.
 * 
 * @param parent			The parent widget.
 * @param bufferRowIndex	The index of the photo in the table buffer.
 * @param newFilepath		The new filepath to set.
 */
void PhotosTable::updateFilepathAt(QWidget& parent, BufferRowIndex bufferRowIndex, QString newFilepath)
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
		
		if (column->type == String && data.toString().isEmpty()) data = QVariant();
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns the translation of "Photo" (singular).
 *
 * @return	The translation of "Photo" (singular).
 */
QString PhotosTable::getItemNameSingular() const
{
	return tr("Photo");
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
 * Returns a translated message confirming that a new photo has been created.
 * 
 * @return	The translated message confirming creation of a new photo.
 */
QString PhotosTable::getCreationConfirmMessage() const
{
	return tr("Saved new photo.");
}

/**
 * Returns a translated message confirming that a number of photos have been edited.
 *
 * @param numEdited	The number of photos that have been edited.
 * @return			The translated message confirming the editing of the photos.
 */
QString PhotosTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln photo(s).", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of photos have been deleted.
 *
 * @param numDeleted	The number of photos that have been deleted.
 * @return				The translated message confirming the deletion of the photos.
 */
QString PhotosTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln photo(s).", "", numDeleted);
}

/**
 * Returns a translated title for a photo filter wizard.
 * 
 * @return	The translated title for a photo filter wizard.
 */
QString PhotosTable::getNewFilterString() const
{
	return tr("New photo filter");
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n photo(s)".
 * 
 * @param numItems	The number of photos to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString PhotosTable::getItemCountString(int numItems) const
{
	return tr("%Ln photo(s)", "", numItems);
}
