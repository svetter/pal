#include "hiker_list.h"



HikersOnAscent::HikersOnAscent() :
		list(QList<QPair<int, QString>>())
{}



void HikersOnAscent::addHiker(Hiker* hiker)
{
	int currentNumHikers = list.size();
	beginInsertRows(QModelIndex(), currentNumHikers, currentNumHikers);
	QPair<int, QString> pair{ hiker->hikerID, hiker->name };
	list.append(pair);
	endInsertRows();
}

void HikersOnAscent::removeHikerAt(int rowIndex)
{
	beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
	list.removeAt(rowIndex);
	endRemoveRows();
}



int HikersOnAscent::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return list.size();
}

int HikersOnAscent::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant HikersOnAscent::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DisplayRole) return QVariant();
	switch (index.column()) {
	case 0:
		return list.at(index.row()).first;
	case 1:
		return list.at(index.row()).second;
	}
	assert(false);
	return QVariant();
}
