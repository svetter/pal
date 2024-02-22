/*
 * Copyright 2023-2024 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file hiker_list.cpp
 * 
 * This file defines the HikersOnAscent class.
 */

#include "hiker_list.h"



/**
 * Creates a new HikersOnAscent object with no hikers.
 */
HikersOnAscent::HikersOnAscent() :
	QAbstractTableModel(),
	list(QList<QPair<ValidItemID, QString>>())
{}



/**
 * Adds a hiker to the list.
 * 
 * @param hiker	The hiker to add.
 */
void HikersOnAscent::addHiker(const Hiker& hiker)
{
	int currentNumHikers = list.size();
	beginInsertRows(QModelIndex(), currentNumHikers, currentNumHikers);
	list.append({FORCE_VALID(hiker.hikerID), hiker.name});
	endInsertRows();
}

/**
 * Removes the hiker at the given row index.
 * 
 * @param rowIndex	The row index of the hiker to remove.
 */
void HikersOnAscent::removeHikerAt(int rowIndex)
{
	beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
	list.removeAt(rowIndex);
	endRemoveRows();
}

/**
 * Removes all hikers from the list.
 */
void HikersOnAscent::clear()
{
	while (rowCount()) removeHikerAt(0);
}



/**
 * Returns whether the list contains the hiker with the given ID.
 * 
 * @param otherHikerID	The ID of the hiker to check for.
 * @return				True if the list contains the hiker with the given ID, false otherwise.
 */
bool HikersOnAscent::containsHiker(ValidItemID otherHikerID) const
{
	for (const auto& [hikerID, name] : list) {
		if (hikerID == otherHikerID) return true;
	}
	return false;
}

/**
 * Returns the set of IDs of the hikers in the list.
 * 
 * @return	The set of IDs of the hikers in the list.
 */
QSet<ValidItemID> HikersOnAscent::getHikerIDSet() const
{
	QSet<ValidItemID> result = QSet<ValidItemID>();
	for (const auto& [hikerID, name] : list) {
		result.insert(hikerID);
	}
	return result;
}



/**
 * For the QAbstractTableModel implementation, returns the number of rows in the list.
 * 
 * @param parent	The parent model index, which is ignored.
 * @return			The number of rows in the list.
 */
int HikersOnAscent::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return list.size();
}

/**
 * For the QAbstractTableModel implementation, returns 2 as the number of columns in the list.
 * 
 * @param parent	The parent model index, which is ignored.
 * @return			The number of columns in the list, which is always 2.
 */
int HikersOnAscent::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

/**
 * For the QAbstractTableModel implementation, returns the data for the given role and model index.
 * 
 * @param index	The model index of the data to return.
 * @param role	The role of the data to return. Everyting except Qt::DisplayRole is ignored.
 * @return		The data for the given role and model index.
 */
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
