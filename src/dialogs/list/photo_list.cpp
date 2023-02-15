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

void PhotosOfAscent::removePhotoAt(int rowIndex)
{
	beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
	list.removeAt(rowIndex);
	endRemoveRows();
}



QStringList PhotosOfAscent::getPhotoList() const
{
	return list;
}



int PhotosOfAscent::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return list.size();
}

QVariant PhotosOfAscent::data(const QModelIndex& index, int role) const
{
	assert(index.column() == 0);
	if (role != Qt::DisplayRole) return QVariant();
	return list.at(index.row());
}


Qt::ItemFlags PhotosOfAscent::flags(const QModelIndex& index) const
{
	Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
	if (index.isValid()) {
		return defaultFlags | Qt::ItemIsDragEnabled;
	} else {
		return defaultFlags | Qt::ItemIsDropEnabled;
	}
}

bool PhotosOfAscent::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role != Qt::DisplayRole) return false;
	list.replace(index.row(), value.toString());
	return true;
}

bool PhotosOfAscent::insertRows(int row, int count, const QModelIndex& parent)
{
	if (parent.isValid()) return false;
	list.insert(row, count, QString());
	return true;
}

bool PhotosOfAscent::removeRows(int row, int count, const QModelIndex& parent)
{
	if (parent.isValid()) return false;
	beginRemoveRows(parent, row, row + count - 1);
	list.remove(row, count);
	endRemoveRows();
	return true;
}

Qt::DropActions PhotosOfAscent::supportedDropActions() const
{
	return Qt::MoveAction;
}
