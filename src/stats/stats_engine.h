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

#include <QObject>
#include <QBoxLayout>



/**
 * A base class for statistical analysis classes, general and specific to each item type.
 */
class StatsEngine : protected QObject
{
	Q_OBJECT
	
protected:
	/** The database. */
	Database* const db;
	
	StatsEngine(Database* db);
	virtual ~StatsEngine();
	
	static void addChartsToLayout(QBoxLayout* layout, const QList<Chart*>& charts, QList<int> stretchFactors = QList<int>());
	
	static QStringList getHistCategories(int increment, int max, QString prefix, QString suffix);
	static int classifyHistValue(int value, int increment, int max);
};



/**
 * A class for general statistical analysis of a project.
 */
class GeneralStatsEngine : public StatsEngine
{
	Q_OBJECT
	
	/** A double pointer to the layout of the statistics tab. */
	QVBoxLayout** const statisticsTabLayoutPtr;
	
	/** A chart showing the elevation gain sum for each year since the first ascent. */
	YearBarChart* elevGainPerYearChart;
	/** A chart showing the number of ascents in each year since the first ascent. */
	YearBarChart* numAscentsPerYearChart;
	/** A chart showing elevation gain and peak height for every logged ascent. */
	TimeScatterChart* heightsScatterChart;
	
public:
	GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr);
	virtual ~GeneralStatsEngine();
	
	void setupStatsTab();
	void resetStatsTab();
	void updateStatsTab();
};



/**
 * A class for statistical analysis related to a specific item type.
 */
class ItemStatsEngine : public StatsEngine
{
	Q_OBJECT
	
	/** The item type to compute and show statistics for and/or about. */
	const PALItemType itemType;
	/** The base table for the item type. */
	const NormalTable* const baseTable;
	/** The layout in which to display the charts. */
	QVBoxLayout* const statsLayout;
	
	/** The value increment between categories in the peak height histogram. */
	const int peakHeightHistCategoryIncrement;
	/** The lower bound of the highest category in the peak height histogram. */
	const int peakHeightHistCategoryMax;
	/** The translated category names for the peak height histogram. */
	const QStringList peakHeightHistCategories;
	/** The number of categories in the peak height histogram. */
	const int numPeakHeightHistCategories;
	
	/** The value increment between categories in the elevation gain histogram. */
	const int elevGainHistCategoryIncrement;
	/** The lower bound of the highest category in the elevation gain histogram. */
	const int elevGainHistCategoryMax;
	/** The translated category names for the elevation gain histogram. */
	const QStringList elevGainHistCategories;
	/** The number of categories in the elevation gain histogram. */
	const int numElevGainHistCategories;
	
	/** A chart showing the distribution of peak heights for the selected items as a histogram. */
	HistogramChart* peakHeightHistChart;
	/** A chart showing the distribution of elevation gains for the selected items as a histogram. */
	HistogramChart* elevGainHistChart;
	/** A chart showing the peak heights and elevation gains for the selected items as a scatterplot. */
	TimeScatterChart* heightsScatterChart;
	
	/** A chart showing the items with the highest number of ascents. */
	TopNChart* topTenNumAscentsChart;
	/** A chart showing the items with the highest maximum peak heights. */
	TopNChart* topTenMaxPeakHeightChart;
	/** A chart showing the items with the highest maximum elevation gains. */
	TopNChart* topTenMaxElevGainChart;
	/** A chart showing the items with the highest elevation gain sums. */
	TopNChart* topTenElevGainSumChart;
	
public:
	ItemStatsEngine(Database* db, PALItemType itemType, const NormalTable* baseTable, QVBoxLayout* statsLayout);
	virtual ~ItemStatsEngine();
	
	void setupStatsPanel();
	void resetStatsPanel();
	void updateStatsPanel(const QSet<BufferRowIndex>& selectedBufferRows);
	
private:
	void updateTopNChart(TopNChart* const chart, const Breadcrumbs& crumbs, const QSet<BufferRowIndex>& selectedBufferRows, std::function<qreal (const QList<BufferRowIndex>&)> valueFromTargetBufferRows) const;
	QString getItemLabelFor(const BufferRowIndex& bufferIndex) const;
};



#endif // STATS_ENGINE_H
