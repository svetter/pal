#ifndef PHOTO_LIST_H
#define PHOTO_LIST_H

#include <QAbstractTableModel>
#include <QStringList>



class PhotosOfAscent : public QAbstractTableModel {
	QStringList list;
	
public:
	PhotosOfAscent();
	
	void addPhotos(const QStringList& photo);
	void removePhoto(const QModelIndex& index);
	
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};



#endif // PHOTO_LIST_H
