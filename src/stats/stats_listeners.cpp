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
 * @file stats_listeners.cpp
 * 
 * This file defines the TableChangeListenerGeneralStatsEngine and
 * TableChangeListenerItemStatsEngine classes.
 */

#include "stats_listeners.h"

#include "src/stats/stats_engine.h"



/**
 * Creates a new TableChangeListenerGeneralStatsEngine.
 * 
 * @param owner	The GeneralStatsEngine that this listener belongs and reports changes to.
 */
TableChangeListenerGeneralStatsEngine::TableChangeListenerGeneralStatsEngine(GeneralStatsEngine& owner) :
	TableChangeListener(),
	owner(owner)
{}

/**
 * Destroys the TableChangeListenerGeneralStatsEngine.
 */
TableChangeListenerGeneralStatsEngine::~TableChangeListenerGeneralStatsEngine()
{}



/**
 * This method is called after any data in the database was changed.
 *
 * @param affectedColumns				The columns whose data has been changed.
 * @param rowsAddedOrRemovedPerTable	The rows that have been added or removed from the table. The bool indicates whether the row was added (true) or removed (false).
 */
void TableChangeListenerGeneralStatsEngine::dataChanged(const QSet<const Column*>& affectedColumns, const QHash<const Table*, QList<QPair<BufferRowIndex, bool>>>& rowsAddedOrRemovedPerTable) const
{
	Q_UNUSED(rowsAddedOrRemovedPerTable);
	
	if (affectedColumns.isEmpty()) return;
	
	const QHash<const Column*, QSet<Chart*>> chartsPerColumn = owner.getAffectedChartsPerColumn();
	for (const Column* column : affectedColumns) {
		if (chartsPerColumn.contains(column)) {
			owner.markChartsDirty(chartsPerColumn.value(column));
		}
	}
}





/**
 * Creates a new TableChangeListenerItemStatsEngine.
 * 
 * @param owner	The ItemStatsEngine that this listener belongs and reports changes to.
 */
TableChangeListenerItemStatsEngine::TableChangeListenerItemStatsEngine(ItemStatsEngine& owner) :
	TableChangeListener(),
	owner(owner)
{}

/**
 * Destroys the TableChangeListenerItemStatsEngine.
 */
TableChangeListenerItemStatsEngine::~TableChangeListenerItemStatsEngine()
{}



/**
 * This method is called after any data in the database was changed.
 *
 * @param affectedColumns				The columns whose data has been changed.
 * @param rowsAddedOrRemovedPerTable	The rows that have been added or removed from the table. The bool indicates whether the row was added (true) or removed (false).
 */
void TableChangeListenerItemStatsEngine::dataChanged(const QSet<const Column*>& affectedColumns, const QHash<const Table*, QList<QPair<BufferRowIndex, bool>>>& rowsAddedOrRemovedPerTable) const
{
	Q_UNUSED(rowsAddedOrRemovedPerTable);
	
	if (affectedColumns.isEmpty()) return;
	
	owner.announceColumnChanges(affectedColumns);
}
