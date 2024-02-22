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
 * @file hiker_list.h
 * 
 * This file declares the HikersOnAscent class.
 */

#ifndef HIKER_LIST_H
#define HIKER_LIST_H

#include "src/data/hiker.h"

#include <QAbstractTableModel>
#include <QList>
#include <QSet>
#include <QPair>
#include <QString>



/**
 * A class encapsulating a list of hikers associated with an ascent.
 * 
 * This class is a model for a QTableView. It is used in the ascent dialog to display the hikers
 * associated with the ascent.
 */
class HikersOnAscent : public QAbstractTableModel {
	/** The list of hikers, each represented by a pair of their ID and their name. */
	QList<QPair<ValidItemID, QString>> list;
	
public:
	HikersOnAscent();
	
	void addHiker(const Hiker& hiker);
	void removeHikerAt(int rowIndex);
	void clear();
	
	bool containsHiker(ValidItemID otherHikerID) const;
	QSet<ValidItemID> getHikerIDSet() const;
	
	// QAbstractTableModel implementation
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};



#endif // HIKER_LIST_H
