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
 * @file photo_list.h
 * 
 * This file declares the PhotosOfAscent class.
 */

#ifndef PHOTO_LIST_H
#define PHOTO_LIST_H

#include "src/data/photo.h"

#include <QAbstractItemModel>
#include <QStringList>



/**
 * A class encapsulating a list of photos associated with an ascent.
 * 
 * This class is a model for a QTableView. It is used in the ascent dialog to display the photos
 * associated with the ascent. It supports drag and drop to enable reordering of the photos.
 */
class PhotosOfAscent : public QAbstractItemModel {
	/** The list of photos. */
	QList<Photo> list;
	
public:
	PhotosOfAscent();
	
	void addPhotos(const QList<Photo>& photo);
	
	bool isEmpty() const;
	const QString& getFilepathAt(int rowIndex) const;
	const QString& getDescriptionAt(int rowIndex) const;
	void setDescriptionAt(int rowIndex, QString description);
	void removePhotoAt(int rowIndex);
	
	QList<Photo> getPhotoList();
	
	// QAbstractItemModel implementation
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;	
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	Qt::DropActions supportedDropActions() const override;
	static const QString MimeType;
	QStringList mimeTypes() const override;
	bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
	QMimeData* mimeData(const QModelIndexList &indexes) const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
};



#endif // PHOTO_LIST_H
