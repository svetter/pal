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
 * @file listeners.h
 * 
 * This file declares the ColumnChangeListener and RowChangeListener classes.
 */

#ifndef LISTENERS_H
#define LISTENERS_H

#include "src/db/row_index.h"



/**
 * A listener for changes in the data of a whole base table column.
 */
class ColumnChangeListener {
public:
	/**
	 * Creates a new ColumnChangeListener.
	 */
	inline ColumnChangeListener()
	{}
	
	/**
	 * Destroys the ColumnChangeListener.
	 */
	virtual inline ~ColumnChangeListener()
	{}
	
	/**
	 * Called when any data in the column owning this listener has changed.
	 */
	virtual void columnDataChanged() const = 0;
};



/**
 * A listener for row changes in a base table, i.e. when a row is inserted or removed.
 */
class RowChangeListener {
public:
	/**
	 * Creates a new RowChangeListener.
	 */
	inline RowChangeListener()
	{}

	/**
	 * Destroys the RowChangeListener.
	 */
	virtual inline ~RowChangeListener()
	{}
	
	/**
	 * Called when a row has been inserted into the base table owning this listener.
	 * 
	 * @param rowIndex	The index of the inserted row.
	 */
	virtual void bufferRowJustInserted(const BufferRowIndex& bufferRowIndex) const = 0;

	/**
	 * Called when a row is about to be removed from the base table owning this listener.
	 * 
	 * @param rowIndex	The index of the row to be removed.
	 */
	virtual void bufferRowAboutToBeRemoved(const BufferRowIndex& bufferRowIndex) const = 0;
};



#endif // LISTENERS_H
