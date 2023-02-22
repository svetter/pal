#include "photo_list.h"

#include <QMimeData>
#include <QIODevice>



PhotosOfAscent::PhotosOfAscent() :
		list(QList<Photo*>())
{}



void PhotosOfAscent::addPhotos(const QList<Photo*>& photos)
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



QList<Photo*> PhotosOfAscent::getPhotoList() const
{
	// Update sorting indices before returning list
	for (int i = 0; list.size(); i++) {
		list.at(i)->photoID = i;
	}
	return QList<Photo*>(list);
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
		return list.at(index.row())->useBasePath;
	case 1:
		return list.at(index.row())->filepath;
	case 2:
		return list.at(index.row())->description;
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
		list.at(index.row())->useBasePath = value.toBool();
		break;
	case 1:
		list.at(index.row())->filepath = value.toString();
		break;
	case 2:
		list.at(index.row())->description = value.toString();
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
	list.insert(row, count, new Photo());
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

bool PhotosOfAscent::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	Q_UNUSED(row);
	Q_UNUSED(column);
	Q_UNUSED(parent);
	if (action != Qt::MoveAction || !data->hasFormat(MimeType))
		return false;
	return true;
}

QMimeData* PhotosOfAscent::mimeData(const QModelIndexList &indexes) const
{
	QMimeData* mimeData = new QMimeData;
	QByteArray encodedData;
	
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	
	for (const QModelIndex& index : indexes) {
		if (!index.isValid()) continue;
		Photo* photo = list.at(index.row());
		
		stream << photo->useBasePath ? "true" : "false";
		stream << photo->filepath;
		stream << photo->description;
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
			newPhotos.append(new Photo());
			newPhotos.last()->useBasePath = text.compare("true", Qt::CaseInsensitive) == 0;
			break;
		case 1:
			newPhotos.last()->filepath = text;
			break;
		case 2:
			newPhotos.last()->description = text;
			break;
		default:
			assert(false);
		}
		
		currentColumn++;
		if (currentColumn == 3) currentColumn = 0;
	}
	
	insertRows(row, newPhotos.size(), QModelIndex());
	for (int rowIndex = row; rowIndex < row + newPhotos.size(); rowIndex++) {
		setData(index(rowIndex, 0, QModelIndex()), newPhotos.at(rowIndex)->useBasePath);
		setData(index(rowIndex, 1, QModelIndex()), newPhotos.at(rowIndex)->filepath);
		setData(index(rowIndex, 2, QModelIndex()), newPhotos.at(rowIndex)->description);
	}
	
	return true;
}
