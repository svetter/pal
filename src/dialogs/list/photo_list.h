#ifndef PHOTO_LIST_H
#define PHOTO_LIST_H

#include <QAbstractListModel>
#include <QStringList>



class PhotosOfAscent : public QAbstractListModel {
	QStringList list;
	
public:
	PhotosOfAscent();
	
	void addPhotos(const QStringList& photo);
	void removePhotoAt(int rowIndex);
	
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	Qt::DropActions supportedDropActions() const override;
};



#endif // PHOTO_LIST_H
