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
 * @file comp_table_listener.h
 * 
 * This file declares the TableChangeListenerCompositeTable class.
 */

#ifndef COMP_TABLE_LISTENER_H
#define COMP_TABLE_LISTENER_H

#include "src/db/table_listener.h"

class CompositeTable;



class TableChangeListenerCompositeTable : public TableChangeListener {
	/** The CompositeTable that this listener belongs and reports changes to. */
	CompositeTable& owner;
	
public:
	TableChangeListenerCompositeTable(CompositeTable& owner);
	virtual ~TableChangeListenerCompositeTable();
	
	virtual void dataChanged(const QSet<const Column*>& affectedColumns, const QHash<const Table*, QList<QPair<BufferRowIndex, bool>>>& rowsAddedOrRemovedPerTable) const;
};



#endif // COMP_TABLE_LISTENER_H
