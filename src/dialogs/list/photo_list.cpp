#include "photo_list.h"



PhotosOfAscent::PhotosOfAscent() :
		list(QStringList())
{}



void PhotosOfAscent::addPhotos(const QStringList& photos)
{
	int currentNumPhotos = list.size();
	beginInsertRows(QModelIndex(), currentNumPhotos, currentNumPhotos + photos.size() - 1);
	list.append(photos);
	endInsertRows();
}

void PhotosOfAscent::removePhoto(const QModelIndex& index)
{
	int rowIndex = index.row();
	beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
	list.removeAt(rowIndex);
	endRemoveRows();
}



int PhotosOfAscent::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return list.size();
}

int PhotosOfAscent::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 1;
}

QVariant PhotosOfAscent::data(const QModelIndex& index, int role) const
{
	assert(index.column() == 0);
	if (role != Qt::DisplayRole) return QVariant();
	return list.at(index.row());
}
