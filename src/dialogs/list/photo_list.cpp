/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file photo_list.cpp
 * 
 * This file defines the PhotosOfAscent class.
 */

#include "photo_list.h"

#include <QMimeData>
#include <QIODevice>



/**
 * Creates a new PhotosOfAscent object with no photos.
 */
PhotosOfAscent::PhotosOfAscent() :
	QAbstractItemModel(),
	list(QList<Photo>())
{}



/**
 * Appends the given photos to the end of the list.
 * 
 * @param photos	The photos to add.
 */
void PhotosOfAscent::addPhotos(const QList<Photo>& photos)
{
	QList<Photo> sortedPhotos = photos;
	auto comparator = [](const Photo& p1, const Photo& p2) { return p1.sortIndex < p2.sortIndex; };
	std::sort(sortedPhotos.begin(), sortedPhotos.end(), comparator);
	
	int currentNumPhotos = list.size();
	beginInsertRows(QModelIndex(), currentNumPhotos, currentNumPhotos + photos.size() - 1);
	for (const Photo& photo : sortedPhotos) {
		list.append(Photo(photo));
	}
	endInsertRows();
}


/**
 * Indicates whether the photos list is currently empty.
 * 
 * @return	True if there are currently no photos in the list, false otherwise.
 */
bool PhotosOfAscent::isEmpty() const
{
	return list.isEmpty();
}

/**
 * Returns the file path of the photo at the given row index.
 * 
 * @param rowIndex	The row index of the photo.
 * @return			The file path of the photo.
 */
const QString& PhotosOfAscent::getFilepathAt(int rowIndex) const
{
	return list[rowIndex].filepath;
}

/**
 * Returns the description of the photo at the given row index.
 * 
 * @param rowIndex	The row index of the photo.
 * @return			The description of the photo.
 */
const QString& PhotosOfAscent::getDescriptionAt(int rowIndex) const
{
	return list[rowIndex].description;
}

/**
 * Sets the description of the photo at the given row index.
 * 
 * @param rowIndex		The row index of the photo.
 * @param description	The new description of the photo.
 */
void PhotosOfAscent::setDescriptionAt(int rowIndex, QString description)
{
	list[rowIndex].description = description;
}

/**
 * Removes the photo at the given row index.
 * 
 * @param rowIndex	The row index of the photo to remove.
 */
void PhotosOfAscent::removePhotoAt(int rowIndex)
{
	beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
	list.removeAt(rowIndex);
	endRemoveRows();
}



/**
 * Returns the current list of photos.
 * 
 * @return	The current list of photos.
 */
QList<Photo> PhotosOfAscent::getPhotoList()
{
	// Update sorting indices before returning list
	for (int i = 0; i < list.size(); i++) {
		list[i].sortIndex = i;
	}
	return QList<Photo>(list);
}



/**
 * For the QAbstraceTableModel implementation, returns a model index for the item at the given
 * location.
 * 
 * @param row		The row index of the item.
 * @param column	The column index of the item.
 * @param parent	The parent model index of the item.
 * @return			A model index for the item.
 */
QModelIndex PhotosOfAscent::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent)) {
		qDebug() << QString("PhotosOfAscent::index() called with unrecognized location: row %1, column %2, parent").arg(row).arg(column) << parent;
		return QModelIndex();
	}
	return createIndex(row, column, nullptr);
}

/**
 * For the QAbstraceTableModel implementation, returns the parent model index of the item at the
 * given location.
 * 
 * @param index	The model index of the item, which is assumed to be valid but otherwise ignored.
 * @return		The parent model index of the item, which is always an invalid model index.
 */
QModelIndex PhotosOfAscent::parent(const QModelIndex& index) const
{
	assert(index.isValid());
	return QModelIndex();
}

/**
 * For the QAbstraceTableModel implementation, returns the number of rows in the list.
 * 
 * @param parent	The parent model index, which is ignored.
 * @return			The number of rows in the list.
 */
int PhotosOfAscent::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return list.size();
}

/**
 * For the QAbstraceTableModel implementation, returns 3 as the number of columns in the list.
 * 
 * @param parent	The parent model index, which is ignored.
 * @return			The number of columns in the list, which is always 3.
 */
int PhotosOfAscent::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 3;
}

/**
 * For the QAbstraceTableModel implementation, returns the data for the given role and model index.
 * 
 * @param index	The model index of the data to return.
 * @param role	The role of the data to return. Everyting except Qt::DisplayRole is ignored.
 * @return		The data for the given role and model index.
 */
QVariant PhotosOfAscent::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DisplayRole) return QVariant();
	switch (index.column()) {
	case 0:
		return list.at(index.row()).filepath;
	case 1:
		return list.at(index.row()).description;
	}
	assert(false);
	return QVariant();
}


/**
 * For the QAbstraceTableModel implementation, returns the Qt::ItemFlags for the item at the given
 * model index.
 * 
 * In addition to the default flags, this method returns Qt::ItemIsDragEnabled for list entries
 * (valid model indices) and Qt::ItemIsDropEnabled for the spaces between list entries (invalid
 * model indices). Dropping *onto* other list entries is therefore not supported.
 * 
 * @param index	The model index of the item.
 * @return		The Qt::ItemFlags for the item.
 */
Qt::ItemFlags PhotosOfAscent::flags(const QModelIndex& index) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
	if (index.isValid()) {
		return defaultFlags | Qt::ItemIsDragEnabled;
	} else {
		return defaultFlags | Qt::ItemIsDropEnabled;
	}
}

/**
 * For the QAbstraceTableModel implementation, sets the data for the given role and model index.
 * 
 * This is used for inserting data during a drag and drop action.
 * 
 * @param index	The model index of the data to set.
 * @param value	The new data.
 * @param role	The role of the data to set. Everyting except Qt::DisplayRole is ignored.
 * @return		True if the data was set successfully, false otherwise.
 */
bool PhotosOfAscent::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role != Qt::DisplayRole) return false;
	
	switch (index.column()) {
	case 0:
		list[index.row()].filepath = value.toString();
		break;
	case 1:
		list[index.row()].description = value.toString();
		break;
	default:
		assert(false);
		return false;
	}
	return true;
}

/**
 * For the QAbstraceTableModel implementation, inserts the given number of empty rows at the given
 * row index.
 * 
 * This is used for inserting new rows during a drag and drop action.
 * 
 * @param row		The row index at which to insert the new rows.
 * @param count		The number of rows to insert.
 * @param parent	The parent model index. Calls with a valid parent model index are ignored.
 * @return			True if the rows were inserted successfully, false otherwise.
 */
bool PhotosOfAscent::insertRows(int row, int count, const QModelIndex& parent)
{
	if (parent.isValid()) return false;
	beginInsertRows(QModelIndex(), row, row + count - 1);
	for (int rowIndex = row; rowIndex < row + count; rowIndex++) {
		list.insert(rowIndex, 1, Photo());
	}
	endInsertRows();
	return true;
}

/**
 * For the QAbstraceTableModel implementation, removes the given number of rows starting at the
 * given row index.
 * 
 * This is used for removing rows during a drag and drop action.
 * 
 * @param row		The index of the first row to remove.
 * @param count		The number of consecutive rows to remove.
 * @param parent	The parent model index. Calls with a valid parent model index are ignored.
 * @return			True if the rows were removed successfully, false otherwise.
 */
bool PhotosOfAscent::removeRows(int row, int count, const QModelIndex& parent)
{
	if (parent.isValid()) return false;
	beginRemoveRows(parent, row, row + count - 1);
	list.remove(row, count);
	endRemoveRows();
	return true;
}

/**
 * For the QAbstraceTableModel implementation, returns the supported Qt::DropActions.
 * 
 * @return	The supported drop actions, which is only Qt::MoveAction.
 */
Qt::DropActions PhotosOfAscent::supportedDropActions() const
{
	return Qt::MoveAction;
}

/** The MIME type string used for drag and drop operations in the list. */
const QString PhotosOfAscent::MimeType = "PeakAscentLogger/Photo";

/**
 * For the QAbstraceTableModel implementation, returns the supported MIME types.
 * 
 * @return	The supported MIME types, which is only PhotosOfAscent::MimeType.
 */
QStringList PhotosOfAscent::mimeTypes() const
{
	QStringList types;
	types << MimeType;
	return types;
}

/**
 * For the QAbstraceTableModel implementation, returns whether the given MIME data can be dropped
 * at the given location.
 * 
 * @param data		The MIME data to drop.
 * @param action	The drop action.
 * @param row		The row index of the drop location, which is ignored.
 * @param column	The column index of the drop location, which is ignored.
 * @param parent	The parent model index of the drop location, which is ignored.
 * @return			True if the given MIME data can be dropped at the given location, false otherwise.
 */
bool PhotosOfAscent::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
	Q_UNUSED(row);
	Q_UNUSED(column);
	Q_UNUSED(parent);
	if (action != Qt::MoveAction || !data->hasFormat(MimeType))
		return false;
	return true;
}

/**
 * For the QAbstraceTableModel implementation, returns the MIME data for the given model indices.
 * 
 * This is used for serializing the data from dragged rows into MIME data during a drag and drop
 * action.
 * 
 * @param indexes	The model indices for which to return the MIME data.
 * @return			The MIME data for the given model indices.
 */
QMimeData* PhotosOfAscent::mimeData(const QModelIndexList& indexes) const
{
	QMimeData* mimeData = new QMimeData;
	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	
	QMap<int, QModelIndex> indexMap = QMap<int, QModelIndex>();
	for (const QModelIndex& index : indexes) {
		if (!index.isValid()) continue;
		indexMap.insert(index.row(), index);
	}
	QList<int> sortedRowList = indexMap.keys();
	
	for (int rowIndex : sortedRowList) {
		Photo photo = list.at(rowIndex);
		
		stream << photo.filepath;
		stream << photo.description;
	}
	mimeData->setData(MimeType, encodedData);
	return mimeData;
}

/**
 * For the QAbstraceTableModel implementation, inserts the data from the given MIME data at the
 * given location.
 * 
 * This is used for deserializing the data from dropped rows from MIME data during a drag and drop
 * action.
 * 
 * @param data		The dropped MIME data.
 * @param action	The drop action.
 * @param row		The row index of the drop location.
 * @param column	The column index of the drop location.
 * @param parent	The parent model index of the drop location.
 * @return			True if the given MIME data was dropped successfully, false otherwise.
 */
bool PhotosOfAscent::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	if (!canDropMimeData(data, action, row, column, parent))
		return false;
	
	if (action == Qt::IgnoreAction)
		return true;
	else if (action != Qt::MoveAction)
		return false;
	
	QByteArray encodedData = data->data(MimeType);
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	QList<Photo> newPhotos = QList<Photo>();
	
	int currentColumn = 0;
	while (!stream.atEnd()) {
		QString text;
		stream >> text;
		
		switch (currentColumn) {
		case 0:
			newPhotos.append(Photo());
			newPhotos.last().filepath = text;
			break;
		case 1:
			newPhotos.last().description = text;
			break;
		default:
			assert(false);
		}
		
		currentColumn++;
		if (currentColumn == 2) currentColumn = 0;
	}
	
	insertRows(row, newPhotos.size(), QModelIndex());
	for (int i = 0; i < newPhotos.size(); i++) {
		setData(index(row + i, 0, QModelIndex()), newPhotos.at(i).filepath, Qt::DisplayRole);
		setData(index(row + i, 1, QModelIndex()), newPhotos.at(i).description, Qt::DisplayRole);
	}
	
	return true;
}
