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



bool HikersOnAscent::containsHiker(int hikerID) const
{
	for (auto iter = list.constBegin(); iter != list.constEnd(); iter++) {
		if ((*iter).first == hikerID) return true;
	}
	return false;
}

QSet<int> HikersOnAscent::getHikerIDSet() const
{
	QSet<int> result = QSet<int>();
	for (auto iter = list.constBegin(); iter != list.constEnd(); iter++) {
		result.insert((*iter).first);
	}
	return result;
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
