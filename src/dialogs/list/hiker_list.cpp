#include "hiker_list.h"



HikersOnAscent::HikersOnAscent() :
		QAbstractTableModel(),
		list(QList<QPair<ValidItemID, QString>>())
{}



void HikersOnAscent::addHiker(Hiker* hiker)
{
	int currentNumHikers = list.size();
	beginInsertRows(QModelIndex(), currentNumHikers, currentNumHikers);
	list.append({hiker->hikerID.forceValid(), hiker->name});
	endInsertRows();
}

void HikersOnAscent::removeHikerAt(int rowIndex)
{
	beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
	list.removeAt(rowIndex);
	endRemoveRows();
}

void HikersOnAscent::clear()
{
	while (rowCount()) removeHikerAt(0);
}



bool HikersOnAscent::containsHiker(ValidItemID hikerID) const
{
	for (auto iter = list.constBegin(); iter != list.constEnd(); iter++) {
		if ((*iter).first == hikerID) return true;
	}
	return false;
}

QSet<ValidItemID> HikersOnAscent::getHikerIDSet() const
{
	QSet<ValidItemID> result = QSet<ValidItemID>();
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
		return list.at(index.row()).first.asQVariant();
	case 1:
		return list.at(index.row()).second;
	}
	assert(false);
	return QVariant();
}
