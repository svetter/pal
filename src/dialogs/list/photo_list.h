#ifndef PHOTO_LIST_H
#define PHOTO_LIST_H

#include "src/data/photo.h"

#include <QAbstractItemModel>
#include <QStringList>



class PhotosOfAscent : public QAbstractItemModel {
	QList<Photo*> list;
	
public:
	PhotosOfAscent();
	
	void addPhotos(const QList<Photo*>& photo);
	void removePhotoAt(int rowIndex);
	
	QList<Photo*> getPhotoList() const;
	
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
	bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
	QMimeData* mimeData(const QModelIndexList &indexes) const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
};



#endif // PHOTO_LIST_H
