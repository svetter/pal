/*
 * Copyright 2023 Simon Vetter
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
 * @file stats_engine.h
 * 
 * This file declares the StatsEngine, GeneralStatsEngine and ItemStatsEngine classes.
 */

#ifndef STATS_ENGINE_H
#define STATS_ENGINE_H

#include "src/db/database.h"

#include "chart.h"
#include "src/data/item_types.h"
#include "src/comp_tables/breadcrumbs.h"

#include <QBoxLayout>



/**
 * A base class for statistical analysis classes, general and specific to each item type.
 */
class StatsEngine
{
protected:
	/** The database. */
	Database* const db;
	
	StatsEngine(Database* db);
	virtual ~StatsEngine();
	
	static void addChartsToLayout(QBoxLayout* layout, const QList<QChartView*>& charts, QList<int> stretchFactors = QList<int>());
};



/**
 * A class for general statistical analysis of a project.
 */
class GeneralStatsEngine : public StatsEngine
{
	/** A double pointer to the layout of the statistics tab. */
	QVBoxLayout** const statisticsTabLayoutPtr;
	
	/** A chart showing the elevation gain sum for each year since the first ascent. */
	YearChart* elevGainPerYearChart;
	/** A chart showing the number of ascents in each year since the first ascent. */
	YearChart* numAscentsPerYearChart;
	/** A chart showing elevation gain and peak height for every logged ascent. */
	YearChart* heightsScatterChart;
	
public:
	GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr);
	virtual ~GeneralStatsEngine();
	
	void setupStatsTab();
	void updateStatsTab();
};



/**
 * A class for statistical analysis related to a specific item type.
 */
class ItemStatsEngine : public StatsEngine
{
	/** The item type to compute and show statistics for and/or about. */
	const PALItemType itemType;
	/** The base table for the item type. */
	const NormalTable* const baseTable;
	/** The layout in which to display the charts. */
	QVBoxLayout* const statsLayout;
	
	/** A chart showing the distribution of peak heights for the selected items as a histogram. */
	HistogramChart* peakHeightHistChart;
	/** A chart showing the distribution of elevation gains for the selected items as a histogram. */
	HistogramChart* elevGainHistChart;
	
public:
	ItemStatsEngine(Database* db, PALItemType itemType, const NormalTable* baseTable, QVBoxLayout* statsFrameLayout);
	virtual ~ItemStatsEngine();
	
	void setupStatsPanel();
	void updateStatsPanel(const QSet<BufferRowIndex>& selectedBufferRows);
};



#endif // STATS_ENGINE_H
