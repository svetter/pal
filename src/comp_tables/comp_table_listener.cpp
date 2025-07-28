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
 * @file comp_table_listener.cpp
 * 
 * This file defines the TableChangeListenerCompositeTable class.
 */

#include "comp_table_listener.h"

#include "composite_table.h"



/**
 * Creates a new TableChangeListenerCompositeTable.
 * 
 * @param owner	The CompositeTable that this listener belongs and reports changes to.
 */
TableChangeListenerCompositeTable::TableChangeListenerCompositeTable(CompositeTable& owner) :
	TableChangeListener(),
	owner(owner)
{}

/**
 * Destroys the TableChangeListenerCompositeTable.
 */
TableChangeListenerCompositeTable::~TableChangeListenerCompositeTable()
{}



/**
 * This method is called after any data in the database was changed.
 *
 * @param affectedColumns				The columns whose data has been changed.
 * @param rowsAddedOrRemovedPerTable	The rows that have been added or removed from the table. The bool indicates whether the row was added (true) or removed (false).
 */
void TableChangeListenerCompositeTable::dataChanged(const QSet<const Column*>& affectedColumns, const QHash<const Table*, QList<QPair<BufferRowIndex, bool>>>& rowsAddedOrRemovedPerTable) const
{
	if (affectedColumns.isEmpty() && rowsAddedOrRemovedPerTable.isEmpty()) return;
	
	const QList<QPair<BufferRowIndex, bool>>& rowsAddedOrRemoved = rowsAddedOrRemovedPerTable.value(&owner.baseTable);
	owner.announceChanges(affectedColumns, rowsAddedOrRemoved);
}
