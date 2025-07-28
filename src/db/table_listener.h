/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file table_listener.h
 * 
 * This file declares the TableChangeListener class.
 */

#ifndef TABLE_LISTENER_H
#define TABLE_LISTENER_H

#include "src/db/row_index.h"

#include <QSet>

class Table;
class Column;



/**
 * A listener for changes in the data of a database table.
 */
class TableChangeListener {
public:
	/**
	 * Creates a new TableChangeListener.
	 */
	inline TableChangeListener()
	{}
	
	/**
	 * Destroys the TableChangeListener.
	 */
	virtual inline ~TableChangeListener()
	{}
	
	/**
	 * This method is called after any data in the database was changed.
	 * 
	 * @param affectedColumns				The columns whose data has been changed.
	 * @param rowsAddedOrRemovedPerTable	The rows that have been added or removed from the table. The bool indicates whether the row was added (true) or removed (false).
	 */
	virtual void dataChanged(const QSet<const Column*>& affectedColumns, const QHash<const Table*, QList<QPair<BufferRowIndex, bool>>>& rowsAddedOrRemovedPerTable) const = 0;
};



#endif // TABLE_LISTENER_H
