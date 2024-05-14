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
 * @file stats_listeners.h
 * 
 * This file declares the TableChangeListenerGeneralStatsEngine and
 * TableChangeListenerItemStatsEngine classes.
 */

#ifndef STATS_LISTENERS_H
#define STATS_LISTENERS_H

#include "src/db/table_listener.h"

class GeneralStatsEngine;
class ItemStatsEngine;



/**
 * A column change listener which notifies a GeneralStatsEngine about changes in an underlying
 * column.
 */
class TableChangeListenerGeneralStatsEngine : public TableChangeListener {
	/** The GeneralStatsEngine that this listener belongs and reports changes to. */
	GeneralStatsEngine& owner;
	
public:
	TableChangeListenerGeneralStatsEngine(GeneralStatsEngine& owner);
	virtual ~TableChangeListenerGeneralStatsEngine();
	
	virtual void dataChanged(const QSet<const Column*>& affectedColumns, const QHash<const Table*, QList<QPair<BufferRowIndex, bool>>>& rowsAddedOrRemovedPerTable) const;
};



/**
 * A column change listener which notifies a ItemStatsEngine about changes in an underlying column.
 */
class TableChangeListenerItemStatsEngine : public TableChangeListener {
	/** The ItemStatsEngine that this listener belongs and reports changes to. */
	ItemStatsEngine& owner;
	
public:
	TableChangeListenerItemStatsEngine(ItemStatsEngine& owner);
	virtual ~TableChangeListenerItemStatsEngine();
	
	virtual void dataChanged(const QSet<const Column*>& affectedColumns, const QHash<const Table*, QList<QPair<BufferRowIndex, bool>>>& rowsAddedOrRemovedPerTable) const;
};



#endif // STATS_LISTENERS_H
