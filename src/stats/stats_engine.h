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

class ColumnChangeListenerItemStatsEngine;



/**
 * A base class for statistical analysis classes, general and specific to each item type.
 */
class StatsEngine : protected QObject
{
	Q_OBJECT
	
	/** Whether the statistics are currently visible to the user and thus need to be kept up to date. */
	bool currentlyVisible;
	
protected:
	/** The database. */
	const Database* const db;
	
	/** A set of all charts managed by this stats engine. */
	QSet<Chart*> charts;
	/** Whether each of the charts is currently dirty and needs to be updated before being shown. */
	QMap<Chart*, bool> dirty;
	
	StatsEngine(Database* db);
	virtual ~StatsEngine();
	
public:
	void setCurrentlyVisible(bool visible, bool noUpdate = false);
	bool isCurrentlyVisible();
	bool anyChartsDirty();
	
protected:
	/**
	 * Regenerates all charts from scratch or from (partial) caches.
	 */
	virtual void updateCharts() = 0;
	
	static void addChartsToLayout(QBoxLayout* layout, const QList<Chart*>& charts, QList<int> stretchFactors = QList<int>());
	
	static QStringList getHistogramClassNames(int increment, int max, QString prefix, QString suffix);
};



/**
 * A class for general statistical analysis of a project.
 */
class GeneralStatsEngine : public StatsEngine
{
	Q_OBJECT
	
	/** A double pointer to the layout of the statistics tab. */
	QVBoxLayout** const statisticsTabLayoutPtr;
	
	/** A chart showing the number of ascents in each year since the first ascent. */
	YearBarChart*		numAscentsPerYearChart;
	/** A chart showing the elevation gain sum for each year since the first ascent. */
	YearBarChart*		elevGainPerYearChart;
	/** A chart showing elevation gain and peak height for every logged ascent. */
	TimeScatterChart*	heightsScatterChart;
	
public:
	GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr);
	virtual ~GeneralStatsEngine();
	
	void setupStatsTab();
	void resetStatsTab();
	void markChartsDirty(const QSet<Chart*>& dirtyCharts);
	
	virtual void updateCharts();
	
protected:
	QHash<Chart*, QSet<Column*>> getUsedColumnSets() const;
private:
	QHash<Column*, QSet<Chart*>> getAffectedChartsPerColumn() const;
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
	
	/** The column change listener used to keep charts up to date. */
	ColumnChangeListenerItemStatsEngine* listener;
	
	// Constants
	/** The number of items to show in the top N charts. */
	static inline const int topN = 10;
	
	// Breadcrumbs
	/** The crumbs from the base table to the ascent table. */
	const Breadcrumbs ascentCrumbs;
	/** The crumbs from the base table to the peak table. */
	const Breadcrumbs peakCrumbs;
	
	// Charts
	/** A chart showing the distribution of peak heights for the selected items as a histogram. */
	HistogramChart*		peakHeightHistChart;
	/** A chart showing the distribution of elevation gains for the selected items as a histogram. */
	HistogramChart*		elevGainHistChart;
	/** A chart showing the peak heights and elevation gains for the selected items as a scatterplot. */
	TimeScatterChart*	heightsScatterChart;
	/** A chart showing the items with the highest number of ascents. */
	TopNChart*			topNumAscentsChart;
	/** A chart showing the items with the highest maximum peak heights. */
	TopNChart*			topMaxPeakHeightChart;
	/** A chart showing the items with the highest maximum elevation gains. */
	TopNChart*			topMaxElevGainChart;
	/** A chart showing the items with the highest elevation gain sums. */
	TopNChart*			topElevGainSumChart;
	
	// Charts source data & state
	/** The current set of buffer rows to build statistics for. */
	QSet<BufferRowIndex> currentStartBufferRows;
	
	// Caching
	// Breadcrumb caches
	/** A cache which holds the results of evaluating the ascent crumbs for individual base table buffer rows. */
	QMap<BufferRowIndex, QList<BufferRowIndex>>	ascentCrumbsResultCache;
	/** A cache which holds the results of evaluating the peak crumbs for individual base table buffer rows. */
	QMap<BufferRowIndex, QList<BufferRowIndex>>	peakCrumbsResultCache;
	// Chart caches
	/** A cache which holds peak height histogram class values for individual peaks table buffer rows. */
	QMap<BufferRowIndex, int>								peakHeightHistCache;
	/** A cache which holds elevation gain histogram class values for individual ascents table buffer rows. */
	QMap<BufferRowIndex, int>								elevGainHistCache;
	/** A cache which holds height scatterplot x and y values for individual ascents table buffer rows. */
	QMap<BufferRowIndex, QPair<QDateTime, QList<qreal>>>	heightsScatterCache;
	/** A cache which holds the associated number of ascents for individual base table buffer rows. */
	QMap<BufferRowIndex, qreal>								topNumAscentsCache;
	/** A cache which holds the associated maximum peak height for individual base table buffer rows. */
	QMap<BufferRowIndex, qreal>								topMaxPeakHeightCache;
	/** A cache which holds the associated maximum elevation gain for individual base table buffer rows. */
	QMap<BufferRowIndex, qreal>								topMaxElevGainCache;
	/** A cache which holds the associated elevation gain sum for individual base table buffer rows. */
	QMap<BufferRowIndex, qreal>								topElevGainSumCache;
	
public:
	ItemStatsEngine(Database* db, PALItemType itemType, const NormalTable* baseTable, QVBoxLayout* statsLayout);
	virtual ~ItemStatsEngine();
	
	void setupStatsPanel();
	void resetStatsPanel();
	void announceColumnChanges(const QSet<const Column*>& changedColumns);
	
	void setStartBufferRows(const QSet<BufferRowIndex>& newBufferRows);
	virtual void updateCharts();
	
private:
	QList<BufferRowIndex> evaluateCrumbsCached(const Breadcrumbs& crumbs, const QSet<BufferRowIndex>& selectedBufferRows, QMap<BufferRowIndex, QList<BufferRowIndex>>& crumbsResultCache) const;
	void updateHistogramChart(HistogramChart* const chart, const QList<BufferRowIndex>& targetBufferRows, std::function<int (const BufferRowIndex&)> histogramClassFromTargetBufferRow, QMap<BufferRowIndex, int>& cache) const;
	void updateTimeScatterChart(TimeScatterChart* const chart, QList<DateScatterSeries*> allSeries, const QList<BufferRowIndex>& targetBufferRows, std::function<QPair<QDateTime, QList<qreal>> (const BufferRowIndex&)> xyValuesFromTargetBufferRow, QMap<BufferRowIndex, QPair<QDateTime, QList<qreal>>>& cache) const;
	void updateTopNChart(TopNChart* const chart, const Breadcrumbs& crumbs, const QSet<BufferRowIndex>& selectedBufferRows, std::function<qreal (const QList<BufferRowIndex>&)> valueFromTargetBufferRows, QMap<BufferRowIndex, qreal>& cache) const;
	
	QString getItemLabelFor(const BufferRowIndex& bufferIndex) const;
	
	void clearBreadcrumbCacheFor(const Breadcrumbs* const breadcrumbs);
	void clearChartCacheFor(Chart* const chart);
	
	QHash<const Breadcrumbs*, QSet<Chart*>> getBreadcrumbDependencyMap() const;
	QHash<Chart*, QSet<Column*>> getPostCrumbsUnderlyingColumnSetPerChart() const;
	QSet<Column*> getItemLabelUnderlyingColumnSet() const;
	QHash<Chart*, QSet<Column*>> getItemLabelUnderlyingColumnSetPerChart() const;
	QSet<Column*> getUsedColumnSet() const;
};



/**
 * A column change listener which notifies a GeneralStatsEngine about changes in an underlying
 * column.
 */
class ColumnChangeListenerGeneralStatsEngine : public ColumnChangeListener {
	/** The ColumnChangeListenerGeneralStatsEngine to notify about column changes. */
	GeneralStatsEngine* const listener;
	/** The charts affected by the column changes. */
	QSet<Chart*> affectedCharts;
	
public:
	ColumnChangeListenerGeneralStatsEngine(GeneralStatsEngine* listener, const QSet<Chart*>& affectedCharts);
	virtual ~ColumnChangeListenerGeneralStatsEngine();
	
	virtual void columnDataChanged(QSet<const Column*> affectedColumns) const;
};



/**
 * A column change listener which notifies a ItemStatsEngine about changes in an underlying column.
 */
class ColumnChangeListenerItemStatsEngine : public ColumnChangeListener {
	/** The ColumnChangeListenerItemStatsEngine to notify about column changes. */
	ItemStatsEngine* const listener;
	
public:
	ColumnChangeListenerItemStatsEngine(ItemStatsEngine* listener);
	virtual ~ColumnChangeListenerItemStatsEngine();
	
	virtual void columnDataChanged(QSet<const Column*> affectedColumns) const;
};



#endif // STATS_ENGINE_H
