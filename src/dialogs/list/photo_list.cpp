#include "photo_list.h"

#include <QMimeData>
#include <QIODevice>



PhotosOfAscent::PhotosOfAscent() :
		QAbstractItemModel(),
		list(QList<Photo>())
{}



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

const QString& PhotosOfAscent::getDescriptionAt(int rowIndex) const
{
	return list[rowIndex].description;
}

void PhotosOfAscent::setDescriptionAt(int rowIndex, QString description)
{
	list[rowIndex].description = description;
}

void PhotosOfAscent::removePhotoAt(int rowIndex)
{
	beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
	list.removeAt(rowIndex);
	endRemoveRows();
}



QList<Photo> PhotosOfAscent::getPhotoList()
{
	// Update sorting indices before returning list
	for (int i = 0; i < list.size(); i++) {
		list[i].sortIndex = i;
	}
	return QList<Photo>(list);
}



QModelIndex PhotosOfAscent::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent)) {
		qDebug() << QString("PhotosOfAscent::index() called with unrecognized location: row %1, column %2, parent").arg(row).arg(column) << parent;
		return QModelIndex();
	}
	return createIndex(row, column, nullptr);
}

QModelIndex PhotosOfAscent::parent(const QModelIndex& index) const
{
	assert(index.isValid());
	return QModelIndex();
}

int PhotosOfAscent::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return list.size();
}

int PhotosOfAscent::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 3;
}

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


Qt::ItemFlags PhotosOfAscent::flags(const QModelIndex& index) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
	if (index.isValid()) {
		return defaultFlags | Qt::ItemIsDragEnabled;
	} else {
		return defaultFlags | Qt::ItemIsDropEnabled;
	}
}

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

const QString PhotosOfAscent::MimeType = "PeakAscentLogger/Photo";

QStringList PhotosOfAscent::mimeTypes() const
{
	QStringList types;
	types << MimeType;
	return types;
}

bool PhotosOfAscent::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
	Q_UNUSED(row);
	Q_UNUSED(column);
	Q_UNUSED(parent);
	if (action != Qt::MoveAction || !data->hasFormat(MimeType))
		return false;
	return true;
}

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
	QList<Photo*> newPhotos = QList<Photo*>();
	
	int currentColumn = 0;
	while (!stream.atEnd()) {
		QString text;
		stream >> text;
		
		switch (currentColumn) {
		case 0:
			newPhotos.last()->filepath = text;
			break;
		case 1:
			newPhotos.last()->description = text;
			break;
		default:
			assert(false);
		}
		
		currentColumn++;
		if (currentColumn == 2) currentColumn = 0;
	}
	
	insertRows(row, newPhotos.size(), QModelIndex());
	for (int i = 0; i < newPhotos.size(); i++) {
		setData(index(row + i, 1, QModelIndex()), newPhotos.at(i)->filepath, Qt::DisplayRole);
		setData(index(row + i, 2, QModelIndex()), newPhotos.at(i)->description, Qt::DisplayRole);
	}
	
	return true;
}
