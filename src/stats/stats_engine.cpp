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
 * @file stats_engine.cpp
 * 
 * This file defines the StatsEngine, GeneralStatsEngine and ItemStatsEngine classes.
 */

#include "stats_engine.h"



/**
 * Creates a StatsEngine.
 * 
 * @param db	The database.
 */
StatsEngine::StatsEngine(Database* db) :
	currentlyVisible(false),
	db(db),
	charts(QSet<Chart*>()),
	dirty(QMap<Chart*, bool>())
{}

/**
 * Destroys the StatsEngine.
 */
StatsEngine::~StatsEngine()
{}



/**
 * Sets the visibility flag and triggers an update via updateCharts() if the statistics tab is now
 * visible.
 * 
 * @param visible	Whether the charts are currently visible.
 * @param noUpdate	Whether to suppress the chart update which can be triggered by the visibility change.
 */
void StatsEngine::setCurrentlyVisible(bool visible, bool noUpdate)
{
	currentlyVisible = visible;
	
	if (currentlyVisible && !noUpdate) {
		updateCharts();
	}
}

/**
 * Indicates whether the charts are currently visible.
 * 
 * @return	Whether the charts are currently visible.
 */
bool StatsEngine::isCurrentlyVisible()
{
	return currentlyVisible;
}

/**
 * Indicates whether any of the charts are currently dirty.
 * 
 * @return	Tre if any of the charts are currently dirty, false otherwise.
 */
bool StatsEngine::anyChartsDirty()
{
	for (Chart* const chart : qAsConst(charts)) {
		if (dirty.contains(chart) && dirty.value(chart) == true) {
			return true;
		}
	}
	return false;
}



/**
 * Adds the given charts to the given layout and sets stretch factors.
 * 
 * @param layout			The layout to add the charts to.
 * @param charts			The charts to add. Can contain nullptr, which will be ignored.
 * @param stretchFactors	The stretch factors to set for the layout. Can be empty, in which case all charts get a stretch factor of 1.
 */
void StatsEngine::addChartsToLayout(QBoxLayout* layout, const QList<Chart*>& charts, QList<int> stretchFactors)
{
	assert(layout);
	
	int numCharts = 0;
	for (Chart* const chart : charts) {
		if (!chart) continue;
		layout->addWidget(chart->getChartView());
		numCharts++;
	}
	
	if (stretchFactors.isEmpty()) stretchFactors = QList<int>(numCharts, 1);
	int stretchIndex = 0;
	for (const int stretchFactor : stretchFactors) {
		layout->setStretch(stretchIndex++, stretchFactor);
	}
}



/**
 * Returns a list of translated class names for the x-axis of a histogram.
 * 
 * @param increment	The value increment between classes.
 * @param max		The lower bound of the highest class.
 * @param prefix	The prefix for the translated class names (before the number).
 * @param suffix	The suffix for the translated class names (after the number).
 * @return			A list of translated class names.
 */
QStringList StatsEngine::getHistogramClassNames(int increment, int max, QString prefix, QString suffix)
{
	QStringList classNames = QStringList("&lt;" + QString::number(increment));
	for (int i = increment; i <= max; i += increment) {
		classNames.append(prefix + QString::number(i) + suffix);
	}
	return classNames;
}





/**
 * Creates a GeneralStatsEngine.
 * 
 * @param db						The database.
 * @param statisticsTabLayoutPtr	A double pointer to the layout of the statistics tab.
 */
GeneralStatsEngine::GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr) :
	StatsEngine(db),
	statisticsTabLayoutPtr(statisticsTabLayoutPtr),
	numAscentsPerYearChart(nullptr),
	elevGainPerYearChart(nullptr),
	heightsScatterChart(nullptr)
{
	assert(statisticsTabLayoutPtr);
}

/**
 * Destroys the GeneralStatsEngine.
 */
GeneralStatsEngine::~GeneralStatsEngine()
{
	delete elevGainPerYearChart;
	delete numAscentsPerYearChart;
	delete heightsScatterChart;
}



/**
 * Creates and initializes the charts for the statistics tab and sets up the layout, populates the
 * set of all charts and marks them as dirty, and registers change listeners.
 */
void GeneralStatsEngine::setupStatsTab()
{
	numAscentsPerYearChart	= new YearBarChart		(tr("Number of ascents per year"),						tr("Number of ascents"));
	elevGainPerYearChart	= new YearBarChart		(tr("Elevation gain sum per year"),						tr("km"));
	heightsScatterChart		= new TimeScatterChart	(tr("All elevation gains and peak heights over time"),	tr("m"));
	
	// Set layout
	QHBoxLayout* statisticsTabUpperLayout = new QHBoxLayout();
	statisticsTabUpperLayout->setSpacing(10);
	statisticsTabUpperLayout->setObjectName("statisticsTabUpperLayout");
	QVBoxLayout* statisticsTabLayout = *statisticsTabLayoutPtr;
	assert(statisticsTabLayout);
	statisticsTabLayout->addLayout(statisticsTabUpperLayout);
	
	addChartsToLayout(statisticsTabUpperLayout, {
		numAscentsPerYearChart,
		elevGainPerYearChart
	});
	addChartsToLayout(statisticsTabLayout, {
		heightsScatterChart
	}, {2, 3});
	
	// Create set of all charts
	charts.insert(numAscentsPerYearChart);
	charts.insert(elevGainPerYearChart);
	charts.insert(heightsScatterChart);
	// Mark all charts as dirty
	for (Chart* const chart : qAsConst(charts)) {
		dirty[chart] = true;
	}
	
	// Create and register change listeners
	const QHash<Column*, QSet<Chart*>> chartsPerColumn = getAffectedChartsPerColumn();
	const QList<Column*> columns = chartsPerColumn.keys();
	for (Column* const column : columns) {
		const QSet<Chart*> affectedCharts = chartsPerColumn.value(column);
		column->registerChangeListener(new ColumnChangeListenerGeneralStatsEngine(this, affectedCharts));
	}
}

/**
 * Resets the charts in the statistics tab and marks them as dirty.
 */
void GeneralStatsEngine::resetStatsTab()
{
	assert(numAscentsPerYearChart);
	assert(elevGainPerYearChart);
	assert(heightsScatterChart);
	
	numAscentsPerYearChart	->reset();
	elevGainPerYearChart	->reset();
	heightsScatterChart		->reset();
	
	setCurrentlyVisible(false);
	for (Chart* const chart : qAsConst(charts)) {
		dirty[chart] = true;
	}
}

/**
 * Sets the dirty flag for the given charts and updates the charts if the stats tab is currently
 * visible.
 * 
 * To be called when the underlying data has changed.
 * 
 * @param dirtyCharts	A set of all charts to be marked dirty.
 */
void GeneralStatsEngine::markChartsDirty(const QSet<Chart*>& dirtyCharts)
{
	for (Chart* const chart : dirtyCharts) {
		dirty[chart] = true;
	}
}



/**
 * Regenerates all charts from scratch if the dirty flag is set.
 */
void GeneralStatsEngine::updateCharts()
{
	if (!anyChartsDirty()) return;
	
	assert(numAscentsPerYearChart);
	assert(elevGainPerYearChart);
	assert(heightsScatterChart);
	
	QList<qreal> elevGainPerYearSeries		= QList<qreal>();
	QList<qreal> numAscentsPerYearSeries	= QList<qreal>();
	DateScatterSeries elevGainSeries	= DateScatterSeries(tr("Elevation gains"),	6,	QScatterSeries::MarkerShapeRotatedRectangle);
	DateScatterSeries peakHeightSeries	= DateScatterSeries(tr("Peak heights"),		6,	QScatterSeries::MarkerShapeTriangle);
	
	
	QMap<int, int> yearElevGainSums	= QMap<int, int>();
	QMap<int, int> yearNumAscents	= QMap<int, int>();
	QDate minDate = QDate();
	QDate maxDate = QDate();
	qreal elevGainPerYearMaxY = 0;
	int numAscentsPerYearMaxY = 0;
	int heightsMaxY = 0;
	
	for (BufferRowIndex bufferIndex = BufferRowIndex(0); bufferIndex.isValid(db->ascentsTable->getNumberOfRows()); bufferIndex++) {
		const QDate date = db->ascentsTable->dateColumn->getValueAt(bufferIndex).toDate();
		if (!date.isValid()) continue;
		
		const int year = date.year();
		if (date < minDate || !minDate.isValid()) minDate = date;
		if (date > maxDate || !maxDate.isValid()) maxDate = date;
		
		QDateTime dateTime;
		if (dirty.value(heightsScatterChart)) {
			QTime time = db->ascentsTable->timeColumn->getValueAt(bufferIndex).toTime();
			if (!time.isValid()) time = QTime(12, 0);
			dateTime = QDateTime(date, time);
		}
		
		if (dirty.value(numAscentsPerYearChart)) {
			yearNumAscents[year]++;
		}
		
		if (dirty.value(elevGainPerYearChart) || dirty.value(heightsScatterChart)) {
			const QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(bufferIndex);
			if (elevGainRaw.isValid()) {
				const int elevGain = elevGainRaw.toInt();
				
				if (dirty.value(elevGainPerYearChart)) {
					yearElevGainSums[year] += elevGain;
				}
				if (dirty.value(heightsScatterChart)) {
					elevGainSeries.data.append({dateTime, elevGain});
					if (elevGain > heightsMaxY) heightsMaxY = elevGain;
				}
			}
		}
		
		if (dirty.value(heightsScatterChart)) {
			const ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(bufferIndex);
			if (peakID.isValid()) {
				QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueFor(FORCE_VALID(peakID));
				if (peakHeightRaw.isValid()) {
					const int peakHeight = peakHeightRaw.toInt();
					peakHeightSeries.data.append({dateTime, peakHeight});
					if (peakHeight > heightsMaxY) heightsMaxY = peakHeight;
				}
			}
		}
	}
	
	const int minYear = minDate.year();
	const int maxYear = maxDate.year();
	
	if (dirty.value(numAscentsPerYearChart) || dirty.value(elevGainPerYearChart)) {
		for (int year = minYear; year <= maxYear; year++) {
			if (dirty.value(numAscentsPerYearChart)) {
				const int numAscents = yearNumAscents.contains(year) ? yearNumAscents[year] : 0;
				numAscentsPerYearSeries.append(numAscents);
				if (numAscents > numAscentsPerYearMaxY) numAscentsPerYearMaxY = numAscents;
			}
			if (dirty.value(elevGainPerYearChart)) {
				const int elevGainSum = yearElevGainSums.contains(year) ? yearElevGainSums[year] : 0;
				const qreal elevGainSumKm = (qreal) elevGainSum / 1000;
				elevGainPerYearSeries.append(elevGainSumKm);
				if (elevGainSumKm > elevGainPerYearMaxY) elevGainPerYearMaxY = elevGainSumKm;
			}
		}
	}
	
	
	if (dirty.value(numAscentsPerYearChart)) {
		numAscentsPerYearChart->updateData(numAscentsPerYearSeries, minYear, maxYear, numAscentsPerYearMaxY, false);
		dirty[numAscentsPerYearChart] = false;
	}
	if (dirty.value(elevGainPerYearChart)) {
		elevGainPerYearChart->updateData(elevGainPerYearSeries, minYear, maxYear, elevGainPerYearMaxY, false);
		dirty[elevGainPerYearChart] = false;
	}
	if (dirty.value(heightsScatterChart)) {
		const QList<DateScatterSeries*> heightsScatterSeries = {&elevGainSeries, &peakHeightSeries};
		heightsScatterChart->updateData(heightsScatterSeries, minDate, maxDate, heightsMaxY, false);
		dirty[heightsScatterChart] = false;
	}
}



/**
 * Returns a set of columns used by this GeneralStatsEngine for all of its charts.
 * 
 * @return	A set of underlying columns for each chart in this stats engine.
 */
QHash<Chart*, QSet<Column*>> GeneralStatsEngine::getUsedColumnSets() const
{
	return {
		{numAscentsPerYearChart, {
			db->ascentsTable->dateColumn
		}},
		{elevGainPerYearChart, {
			db->ascentsTable->dateColumn,
			db->ascentsTable->elevationGainColumn
		}},
		{heightsScatterChart, {
			db->ascentsTable->dateColumn,
			db->ascentsTable->timeColumn,
			db->ascentsTable->elevationGainColumn,
			db->ascentsTable->peakIDColumn,
			db->peaksTable->heightColumn
		}}
	};
}


/**
 * Returns a map of columns and sets of charts affected by changes to each column.
 * 
 * @return	A map of columns and sets of charts affected by changes to each column.
 */
QHash<Column*, QSet<Chart*>> GeneralStatsEngine::getAffectedChartsPerColumn() const
{
	auto usedColumnsPerChart = getUsedColumnSets();
	QHash<Column*, QSet<Chart*>> chartsPerColumn = QHash<Column*, QSet<Chart*>>();
	
	for (auto iter = usedColumnsPerChart.constBegin(); iter != usedColumnsPerChart.constEnd(); iter++) {
		Chart* const chart = iter.key();
		const QSet<Column*>& columns = iter.value();
		for (Column* const column : columns) {
			chartsPerColumn[column].insert(chart);
		}
	}
	
	return chartsPerColumn;
}





/**
 * Creates an ItemStatsEngine.
 * 
 * @param db			The database.
 * @param itemType		The item type to compute and show statistics for and/or about.
 * @param baseTable		The base table for the item type.
 * @param statsLayout	The layout in which to display the charts.
 */
ItemStatsEngine::ItemStatsEngine(Database* db, PALItemType itemType, const NormalTable* baseTable, QVBoxLayout* statsLayout) :
	StatsEngine(db),
	itemType	(itemType),
	baseTable	(baseTable),
	statsLayout	(statsLayout),
	ascentCrumbs	(db->getBreadcrumbsFor(baseTable, db->ascentsTable)),
	peakCrumbs		(db->getBreadcrumbsFor(baseTable, db->peaksTable)),
	peakHeightHistChart		(nullptr),
	elevGainHistChart		(nullptr),
	heightsScatterChart		(nullptr),
	topNumAscentsChart		(nullptr),
	topMaxPeakHeightChart	(nullptr),
	topMaxElevGainChart		(nullptr),
	topElevGainSumChart		(nullptr),
	currentStartBufferRows	(QSet<BufferRowIndex>()),
	currentlyAllRowsSelected(false),
	ascentCrumbsSingleRowResultCache	(QMap<BufferRowIndex, QList<BufferRowIndex>>()),
	ascentCrumbsWholeSetResultCache		(QHash<QSet<BufferRowIndex>, QList<BufferRowIndex>>()),
	peakCrumbsSingleRowResultCache		(QMap<BufferRowIndex, QList<BufferRowIndex>>()),
	peakCrumbsWholeSetResultCache		(QHash<QSet<BufferRowIndex>, QList<BufferRowIndex>>()),
	peakHeightHistCache		(QMap<BufferRowIndex, int>()),
	elevGainHistCache		(QMap<BufferRowIndex, int>()),
	heightsScatterCache		(QMap<BufferRowIndex, QPair<QDateTime, QList<qreal>>>()),
	topNumAscentsCache		(QMap<BufferRowIndex, qreal>()),
	topMaxPeakHeightCache	(QMap<BufferRowIndex, qreal>()),
	topMaxElevGainCache		(QMap<BufferRowIndex, qreal>()),
	topElevGainSumCache		(QMap<BufferRowIndex, qreal>())
{
	assert(statsLayout);
	
	// Create and register change listeners
	listener = new ColumnChangeListenerItemStatsEngine(this);
	const QSet<Column*> underlyingColumns = getUsedColumnSet();
	for (Column* const column : underlyingColumns) {
		column->registerChangeListener(listener);
	}
}

/**
 * Destroys the ItemStatsEngine.
 */
ItemStatsEngine::~ItemStatsEngine()
{
	delete peakHeightHistChart;
	delete elevGainHistChart;
	delete heightsScatterChart;
	if (itemType != ItemTypeAscent) delete topNumAscentsChart;
	delete topMaxPeakHeightChart;
	delete topMaxElevGainChart;
	if (itemType != ItemTypeAscent) delete topElevGainSumChart;
	
	delete listener;
}



/**
 * Creates and initializes the charts for the statistics panel, populates the set of all charts and
 * marks them as dirty.
 */
void ItemStatsEngine::setupStatsPanel()
{
	{
		QString chartTitle		= tr("Distribution of peak heights");
		int classIncrement		= 1000;
		int classMax			= 8848;
		QStringList classNames	= getHistogramClassNames(classIncrement, classMax, "", tr("s"));
		int numClasses			= classNames.size();
		peakHeightHistChart = new HistogramChart(chartTitle, numClasses, classIncrement, classMax, classNames);
	}
	
	{
		QString chartTitle		= tr("Distribution of elevation gains");
		int classIncrement		= 250;
		int classMax			= 1500;
		QStringList classNames	= getHistogramClassNames(classIncrement, classMax, "&ge;", "");
		int numClasses			= classNames.size();
		elevGainHistChart = new HistogramChart(chartTitle, numClasses, classIncrement, classMax, classNames);
	}
	
	heightsScatterChart = new TimeScatterChart(tr("Elevation gains and peak heights over time"));
	
	if (itemType != ItemTypeAscent) {
		topNumAscentsChart	= new TopNChart(topN, tr("Top %1: Most ascents").arg(topN));
	}
	topMaxPeakHeightChart	= new TopNChart(topN, tr("Top %1: Highest peak").arg(topN));
	topMaxElevGainChart		= new TopNChart(topN, tr("Top %1: Highest single elevation gain").arg(topN));
	if (itemType != ItemTypeAscent) {
		topElevGainSumChart	= new TopNChart(topN, tr("Top %1: Highest elevation gain sum [km]").arg(topN));
	}
	
	heightsScatterChart->getChartView()->setMinimumHeight(250);
	
	addChartsToLayout(statsLayout, {
		peakHeightHistChart,
		elevGainHistChart,
		heightsScatterChart,
		topNumAscentsChart,
		topMaxPeakHeightChart,
		topMaxElevGainChart,
		topElevGainSumChart
	});
	
	// Create set of all charts
	charts.insert(peakHeightHistChart);
	charts.insert(elevGainHistChart);
	charts.insert(heightsScatterChart);
	charts.insert(topNumAscentsChart);
	charts.insert(topMaxPeakHeightChart);
	charts.insert(topMaxElevGainChart);
	charts.insert(topElevGainSumChart);
	charts.remove(nullptr);
	// Mark all charts as dirty
	for (Chart* const chart : qAsConst(charts)) {
		dirty[chart] = true;
	}
}

/**
 * Resets the charts in the statistics panel.
 */
void ItemStatsEngine::resetStatsPanel()
{
	peakHeightHistChart->reset();
	elevGainHistChart->reset();
	heightsScatterChart->reset();
	if (topNumAscentsChart) topNumAscentsChart->reset();
	topMaxPeakHeightChart->reset();
	topMaxElevGainChart->reset();
	if (topElevGainSumChart) topElevGainSumChart->reset();
	
	setCurrentlyVisible(false);
	
	ascentCrumbsSingleRowResultCache	.clear();
	ascentCrumbsWholeSetResultCache		.clear();
	peakCrumbsSingleRowResultCache		.clear();
	peakCrumbsWholeSetResultCache		.clear();
	
	peakHeightHistCache		.clear();
	elevGainHistCache		.clear();
	heightsScatterCache		.clear();
	topNumAscentsCache		.clear();
	topMaxPeakHeightCache	.clear();
	topMaxElevGainCache		.clear();
	topElevGainSumCache		.clear();
	
	// Mark all charts as dirty
	for (Chart* const chart : qAsConst(charts)) {
		dirty[chart] = true;
	}
}

/**
 * Depending on which columns have changed, resets affected caches and marks affected charts as
 * dirty.
 * 
 * To be called when the underlying data has changed.
 */
void ItemStatsEngine::announceColumnChanges(const QSet<const Column*>& changedColumns)
{
	auto intersect = [](const QSet<Column*>& set1, const QSet<const Column*>& set2) {
		for (const Column* const column : set1) {
			if (set2.contains(column)) return true;
		}
		return false;
	};
	
	// === LEVEL 1 ===
	// Changes under breadcrumbs always require breadcrumb and derivative chart cache reset
	const QHash<const Breadcrumbs*, QSet<Chart*>> breadcrumbDependencies = getBreadcrumbDependencyMap();
	const QList<const Breadcrumbs*> allBreadcrumbs = breadcrumbDependencies.keys();
	for (const Breadcrumbs* const breadcrumbs : allBreadcrumbs) {
		if (!intersect(breadcrumbs->getColumnSet(), changedColumns)) continue;
		
		clearBreadcrumbCachesFor(breadcrumbs);
		
		// Mark all dependent charts dirty and reset their caches
		const QSet<Chart*> charts = breadcrumbDependencies.value(breadcrumbs);
		for (Chart* const chart : charts) {
			dirty[chart] = true;
			clearChartCacheFor(chart);
		}
	}
	
	// === LEVEL 2 ===
	// Changes under columns used for chart-specific caches require resetting those caches
	const QHash<Chart*, QSet<Column*>> columnsPerChart = getPostCrumbsUnderlyingColumnSetPerChart();
	QList<Chart*> charts = columnsPerChart.keys();
	for (Chart* const chart : charts) {
		const QSet<Column*> columns = columnsPerChart.value(chart);
		if (!intersect(columns, changedColumns)) continue;
		
		dirty[chart] = true;
		clearChartCacheFor(chart);
	}
	
	// === LEVEL 3 ===
	// Changes under columns used only for item labels only require chart regeneration without any cache resets (only relevant for top-n charts)
	const QHash<Chart*, QSet<Column*>> labelColumnsPerChart = getItemLabelUnderlyingColumnSetPerChart();
	charts = labelColumnsPerChart.keys();
	for (Chart* const chart : charts) {
		const QSet<Column*> columns = labelColumnsPerChart.value(chart);
		if (!intersect(columns, changedColumns)) continue;
		
		dirty[chart] = true;
	}
}



/**
 * Sets the buffer row set on which to base the statistics and triggers an update of the charts if
 * the set has changed.
 * 
 * @param newBufferRows	The buffer rows of all items currently selected in the UI table.
 * @param allRows		Whether all displayed rows are currently selected in the table.
 */
void ItemStatsEngine::setStartBufferRows(const QSet<BufferRowIndex>& newBufferRows, bool allRows)
{
	if (currentStartBufferRows == newBufferRows && currentlyAllRowsSelected == allRows) return;
	
	currentStartBufferRows = newBufferRows;
	currentlyAllRowsSelected = allRows;
	
	for (Chart* const chart : qAsConst(charts)) {
		dirty[chart] = true;
	}
	if (isCurrentlyVisible()) updateCharts();
}

/**
 * Sets the ranges pinned flag for all charts.
 * 
 * @param rangesPinned	Whether to pin the ranges of all charts.
 */
void ItemStatsEngine::setRangesPinned(bool rangesPinned)
{
	peakHeightHistChart		->setUsePinnedRanges(rangesPinned);
	elevGainHistChart		->setUsePinnedRanges(rangesPinned);
	heightsScatterChart		->setUsePinnedRanges(rangesPinned);
	if (topNumAscentsChart) {
		topNumAscentsChart	->setUsePinnedRanges(rangesPinned);
	}
	topMaxPeakHeightChart	->setUsePinnedRanges(rangesPinned);
	topMaxElevGainChart		->setUsePinnedRanges(rangesPinned);
	if (topElevGainSumChart) {
		topElevGainSumChart	->setUsePinnedRanges(rangesPinned);
	}
}

/**
 * Regenerates all charts from scratch or from (partial) caches if the dirty flag is set.
 * 
 * @param selectedBufferRows	The buffer rows of all items currently selected in the UI table.
 */
void ItemStatsEngine::updateCharts()
{
	if (!anyChartsDirty()) return;
	
	assert(peakHeightHistChart);
	assert(elevGainHistChart);
	assert(heightsScatterChart);
	assert((topNumAscentsChart != nullptr) != (itemType == ItemTypeAscent));
	assert(topMaxPeakHeightChart);
	assert(topMaxElevGainChart);
	assert((topElevGainSumChart != nullptr) != (itemType == ItemTypeAscent));
	
	
	// Collect peak/ascent IDs
	
	QList<BufferRowIndex> ascentBufferRows	= evaluateCrumbsCached(ascentCrumbs,	currentStartBufferRows,	ascentCrumbsSingleRowResultCache,	ascentCrumbsWholeSetResultCache);
	QList<BufferRowIndex> peakBufferRows	= evaluateCrumbsCached(peakCrumbs,		currentStartBufferRows,	peakCrumbsSingleRowResultCache,		peakCrumbsWholeSetResultCache);
	
	
	// Peak height histogram
	
	if (peakHeightHistChart) {
		auto peakHeightClassFromPeakBufferRow = [this](const BufferRowIndex& peakBufferRow) {
			const QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueAt(peakBufferRow);
			if (!peakHeightRaw.isValid()) return -1;
			
			const int peakHeight = peakHeightRaw.toInt();
			return peakHeightHistChart->classifyValue(peakHeight);
		};
		
		updateHistogramChart(peakHeightHistChart, peakBufferRows, peakHeightClassFromPeakBufferRow, peakHeightHistCache);
		dirty[peakHeightHistChart] = false;
	}
	
	
	// Elevation gain histogram
	
	if (elevGainHistChart) {
		auto elevGainClassFromAscentBufferRow = [this](const BufferRowIndex& ascentBufferRow) {
			const QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(ascentBufferRow);
			if (!elevGainRaw.isValid()) return -1;
			
			const int elevGain = elevGainRaw.toInt();
			return elevGainHistChart->classifyValue(elevGain);
		};
		
		updateHistogramChart(elevGainHistChart, ascentBufferRows, elevGainClassFromAscentBufferRow, elevGainHistCache);
		dirty[elevGainHistChart] = false;
	}
	
	
	// Heights scatterplot
	
	if (heightsScatterChart) {
		DateScatterSeries elevGainScatterSeries		= DateScatterSeries(tr("Elevation gains"),	8,	QScatterSeries::MarkerShapeRotatedRectangle);
		DateScatterSeries peakHeightScatterSeries	= DateScatterSeries(tr("Peak heights"),		8,	QScatterSeries::MarkerShapeTriangle);
		
		auto xyValuesFromTargetBufferRow = [this](const BufferRowIndex& ascentBufferIndex) {
			const QDate date = db->ascentsTable->dateColumn->getValueAt(ascentBufferIndex).toDate();
			if (!date.isValid()) return QPair<QDateTime, QList<qreal>>();
			
			QTime time = db->ascentsTable->timeColumn->getValueAt(ascentBufferIndex).toTime();
			if (!time.isValid()) time = QTime(12, 0);
			const QDateTime dateTime = QDateTime(date, time);
			
			qreal elevGain		= -1;
			qreal peakHeight	= -1;
			
			const QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(ascentBufferIndex);
			if (elevGainRaw.isValid()) {
				elevGain = elevGainRaw.toInt();
			}
			
			const ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(ascentBufferIndex);
			if (peakID.isValid()) {
				const QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueFor(FORCE_VALID(peakID));
				if (peakHeightRaw.isValid()) {
					peakHeight = peakHeightRaw.toInt();
				}
			}
			
			return QPair<QDateTime, QList<qreal>>(dateTime, {elevGain, peakHeight});
		};
		
		QList<DateScatterSeries*> seriesList = {&elevGainScatterSeries, &peakHeightScatterSeries};
		updateTimeScatterChart(heightsScatterChart, seriesList, ascentBufferRows, xyValuesFromTargetBufferRow, heightsScatterCache);
		dirty[heightsScatterChart] = false;
	}
	
	
	// Top N with most ascents chart
	
	if (topNumAscentsChart) {
		assert(itemType != ItemTypeAscent);
		
		auto numAscentsFromAscentBufferRows = [](const QList<BufferRowIndex>& ascentBufferRows) {
			return ascentBufferRows.size();
		};
		
		updateTopNChart(topNumAscentsChart, ascentCrumbs, currentStartBufferRows, numAscentsFromAscentBufferRows, topNumAscentsCache);
		dirty[topNumAscentsChart] = false;
	}
	
	
	// Top N with highest peaks chart
	
	if (topMaxPeakHeightChart) {
		auto maxPeakHeightFromPeakBufferRows = [this](const QList<BufferRowIndex>& peakBufferRows) {
			int maxPeakHeight = 0;
			for (const BufferRowIndex& peakBufferRow : peakBufferRows) {
				QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueAt(peakBufferRow);
				if (!peakHeightRaw.isValid()) continue;
				
				int peakHeight = peakHeightRaw.toInt();
				if (peakHeight > maxPeakHeight) maxPeakHeight = peakHeight;
			}
			return maxPeakHeight;
		};
		
		updateTopNChart(topMaxPeakHeightChart, peakCrumbs, currentStartBufferRows, maxPeakHeightFromPeakBufferRows, topMaxPeakHeightCache);
		dirty[topMaxPeakHeightChart] = false;
	}
	
	
	// Top N with highest single elevation gain chart
	
	if (topMaxElevGainChart) {
		auto maxElevGainFromAscentBufferRows = [this](const QList<BufferRowIndex>& ascentBufferRows) {
			int maxElevGain = 0;
			for (const BufferRowIndex& ascentBufferRow : ascentBufferRows) {
				QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(ascentBufferRow);
				if (!elevGainRaw.isValid()) continue;
				
				int elevGain = elevGainRaw.toInt();
				if (elevGain > maxElevGain) maxElevGain = elevGain;
			}
			return maxElevGain;
		};
		
		updateTopNChart(topMaxElevGainChart, ascentCrumbs, currentStartBufferRows, maxElevGainFromAscentBufferRows, topMaxElevGainCache);
		dirty[topMaxElevGainChart] = false;
	}
	
	
	// Top N with highest elevation gain sum chart
	
	if (topElevGainSumChart) {
		assert(itemType != ItemTypeAscent);
		
		auto elevGainSumFromAscentBufferRows = [this](const QList<BufferRowIndex>& ascentBufferRows) {
			int elevGainSum = 0;
			for (const BufferRowIndex& ascentBufferRow : ascentBufferRows) {
				QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(ascentBufferRow);
				if (!elevGainRaw.isValid()) continue;
				
				int elevGain = elevGainRaw.toInt();
				elevGainSum += elevGain;
			}
			return (qreal) elevGainSum / 1000;
		};
		
		updateTopNChart(topElevGainSumChart, ascentCrumbs, currentStartBufferRows, elevGainSumFromAscentBufferRows, topElevGainSumCache);
		dirty[topElevGainSumChart] = false;
	}
}


/**
 * Returns the evaluation of the given breadcrumbs for the given selected buffer rows, using or
 * updating the given cache.
 * 
 * Duplication in breadcrumb evaluation
 * 
 * While following a breadcrumb trail, the algorithm can come across duplicate entries. Where this
 * can occur depends on the type of crumb as well as the number of starting items (buffer rows):
 * ┌──────────────────────────┬───────────────────┬───────────────────┐
 * │           Starting from: │    Single item    │      M items      │
 * ├──────────────────────────┼───────────┬───────┼───────────┬───────┤
 * │                          │ # Results │ Dupl. │ # Results │ Dupl. │
 * ├──────────────────────────┼───────────┼───────┼───────────┼───────┤
 * │ Forward reference        │ 0 - 1     │ No    │ 0 - M     │ Yes   │
 * │ Backward reference       │ 0 - n     │ No    │ 0 - n     │ No    │
 * │ Across associative table │ 0 - n     │ No    │ 0 - n     │ Yes   │
 * └──────────────────────────┴───────────┴───────┴───────────┴───────┘
 * When following a trail which is composed ONLY of forward-referencing crumbs, there can only be
 * zero or one results when starting with a single item, while starting with multiple items can
 * lead to duplicated items on the target side.
 * Any trail composed only of backward-referencing crumbs will not produce additional duplication,
 * even when starting with multiple items. This is because following a backwards reference means
 * traversing a 1-to-n relation between classes, and this process could only add duplication if an
 * item on the n side corresponded to multiple items on the 1 side, which it cannot do by
 * definition.
 * Lastly, when traversing an associative table, duplicate results can occur, but only when starting
 * from multiple elements. Duplication from a single element would require multiple entries for one
 * key where the other key is also the same, which is not possible since both keys combine to form
 * the primary key for the associative table. Traversing an associative table amounts to a backward
 * reference followed by a forward reference, which fits with the above observations.
 * 
 * For any trail where duplication can occur, it needs to be determined in which mode breadcrumb
 * steps should be evaluated. There are two possibilities for every crumb in a trail:
 * 1. Discard all duplicates (by using a set).
 * 2. Record every row, even if it has already been added (by using a list).
 * 
 * Regarding the database scheme in this project more specifically, there are four trails which need
 * to be evaluated in this method where duplication can occur:
 * 1. Ascent -> peak
 *    Trivial because the trail only consists of one step. Duplication is wanted and can be
 *    performed after separate evaluation of single items.
 * 2. Trip   -> peak
 *    Duplication only possible during the second of two steps. Again, duplication is wanted and is
 *    compatible with separate evaluation of single items.
 * 3. Hiker  -> ascent
 *    Duplication is NOT wanted here but can be performed at first and discarded after the fact.
 * 4. Hiker  -> peak
 *    Duplication is not wanted for the step from hiker to ascent, but IS wanted for the last step
 *    from ascent to peak. This is NOT compatible with separate evaluation of single items. Instead,
 *    the first step needs to be performed with ALL starting items while ignoring or discarding
 *    duplicates, followed by the second step where duplicates are now recorded.
 * 
 * In order to satisfy these constraints, the solution employed here is to always work with lists to
 * record all duplicates, EXCEPT when traversing an associative table, where duplication is
 * circumvented ONLY for that step, also throwing away any previous duplicates.
 * 
 * @param crumbs						The breadcrumbs to evaluate.
 * @param selectedBufferRows			The buffer rows of all items currently selected in the UI table.
 * @param crumbsSingleRowResultCache	The cache to use for evaluation results of single buffer rows.
 * @param crumbsWholeSetResultCache		The cache to use for evaluation results of entire sets of buffer rows.
 * @return								The evaluation of the given breadcrumbs for the given selected buffer rows.
 */
QList<BufferRowIndex> ItemStatsEngine::evaluateCrumbsCached(const Breadcrumbs& crumbs, const QSet<BufferRowIndex>& selectedBufferRows, QMap<BufferRowIndex, QList<BufferRowIndex>>& crumbsSingleRowResultCache, QHash<QSet<BufferRowIndex>, QList<BufferRowIndex>>& crumbsWholeSetResultCache) const
{
	QList<BufferRowIndex> targetBufferRows = QList<BufferRowIndex>();
	
	if (crumbs.goesVia(db->participatedTable)) {
		// Crumbs traverse an associative table, can't use caches for separately evaluated single rows
		// Use cache for whole set of requested buffer rows instead
		
		// Check cache
		if (crumbsWholeSetResultCache.contains(selectedBufferRows)) {
			// Cache hit
			targetBufferRows = crumbsWholeSetResultCache.value(selectedBufferRows);
		}
		else {
			// Cache miss
			targetBufferRows = crumbs.evaluateForStats(selectedBufferRows);
			// Write to cache
			crumbsWholeSetResultCache.insert(selectedBufferRows, targetBufferRows);
		}
		
		return targetBufferRows;
	}
	
	for (const BufferRowIndex& currentBufferRow : selectedBufferRows) {
		QList<BufferRowIndex> newTargetBufferRows;
		
		// Check cache
		if (crumbsSingleRowResultCache.contains(currentBufferRow)) {
			// Cache hit
			newTargetBufferRows = crumbsSingleRowResultCache.value(currentBufferRow);
		}
		else {
			// Cache miss
			newTargetBufferRows = crumbs.evaluateForStats({ currentBufferRow });
			// Write to cache
			crumbsSingleRowResultCache.insert(currentBufferRow, newTargetBufferRows);
		}
		
		targetBufferRows.append(newTargetBufferRows);
	}
	
	return targetBufferRows;
}


/**
 * Compiles data for an update of a histogram chart and updates it, using or updating the given
 * cache.
 * 
 * @param chart								The chart to update.
 * @param targetBufferRows					The buffer rows of the target table which are associated with the relevant base table rows, in other words, the result of breadcruumb evaluation.
 * @param histogramClassFromTargetBufferRow	A function which returns the histogram class index for a given buffer row in the target table.
 * @param cache								The cache to use.
 */
void ItemStatsEngine::updateHistogramChart(HistogramChart* const chart, const QList<BufferRowIndex>& targetBufferRows, std::function<int (const BufferRowIndex&)> histogramClassFromTargetBufferRow, QMap<BufferRowIndex, int>& cache) const
{
	assert(chart);
	assert(histogramClassFromTargetBufferRow);
	
	QList<qreal> histogramData = QList<qreal>(chart->numClasses, 0);
	qreal maxY = 0;
	
	for (const BufferRowIndex& targetBufferRow : targetBufferRows) {
		int histogramClass;
		
		// Check cache
		if (cache.contains(targetBufferRow)) {
			// Cache hit
			histogramClass = cache.value(targetBufferRow);
		}
		else {
			// Cache miss
			histogramClass = histogramClassFromTargetBufferRow(targetBufferRow);
			
			// Write class to cache
			cache.insert(targetBufferRow, histogramClass);
		}
		
		if (histogramClass < 0) continue;
		
		const qreal newClassCount = ++histogramData[histogramClass];
		if (newClassCount > maxY) maxY = newClassCount;
	}
	
	chart->updateData(histogramData, maxY, currentlyAllRowsSelected);
}

/**
 * Compiles data for an update of a time scatter chart and updates it, using or updating the given
 * cache.
 *
 * @param chart							The chart to update.
 * @param allSeries						A prepared list of DateScatterSeries to use for the chart update. It is assumed that the series are empty, but all configuration values are set.
 * @param targetBufferRows				The buffer rows of the target table which are associated with the relevant base table rows, in other words, the result of breadcruumb evaluation.
 * @param xyValuesFromTargetBufferRow	A function which returns a date and a list of y values for a given buffer row in the target table.
 * @param cache							The cache to use.
 */
void ItemStatsEngine::updateTimeScatterChart(TimeScatterChart* const chart, QList<DateScatterSeries*> allSeries, const QList<BufferRowIndex>& targetBufferRows, std::function<QPair<QDateTime, QList<qreal>> (const BufferRowIndex&)> xyValuesFromTargetBufferRow, QMap<BufferRowIndex, QPair<QDateTime, QList<qreal>>>& cache) const
{
	assert(chart);
	
	QDate minDate = QDate();
	QDate maxDate = QDate();
	int maxY = 0;
	
	for (const BufferRowIndex& targetBufferIndex : targetBufferRows) {
		QDateTime dateTime;
		QDate date;
		QList<qreal> yValues = QList<qreal>(allSeries.size(), -1);
		
		// Check cache
		if (cache.contains(targetBufferIndex)) {
			// Cache hit
			const QPair<QDateTime, QList<qreal>>& cached = cache.value(targetBufferIndex);
			dateTime = cached.first;
			if (!dateTime.isValid()) continue;
			
			yValues = cached.second;
			assert(yValues.size() == allSeries.size());
		}
		else {
			// Cache miss
			QPair<QDateTime, QList<qreal>> xyValues = xyValuesFromTargetBufferRow(targetBufferIndex);
			dateTime = xyValues.first;
			if (!dateTime.isValid()) continue;
			
			yValues = xyValues.second;
			assert(yValues.size() == allSeries.size());
			
			// Write to cache
			cache.insert(targetBufferIndex, {dateTime, yValues});
		}
		
		// Append data and update minima/maxima
		for (int i = 0; i < allSeries.size(); i++) {
			const int yValue = yValues.at(i);
			if (yValue == -1) continue;
			
			allSeries[i]->data.append({dateTime, yValue});
			if (yValue > maxY) maxY = yValue;
		}
		
		date = dateTime.date();
		if (date < minDate || !minDate.isValid()) minDate = date;
		if (date > maxDate || !maxDate.isValid()) maxDate = date;
	}
	
	chart->updateData(allSeries, minDate, maxDate, maxY, currentlyAllRowsSelected);
}

/**
 * Compiles data for an update of a top N chart and updates it.
 * 
 * @param chart						The chart to update.
 * @param crumbs					The breadcrumbs leading to the target table containing the data to be compared.
 * @param selectedBufferRows		The buffer rows of all items currently selected in the UI table.
 * @param valueFromTargetBufferRows	A function which returns a chart value for a given list of buffer rows in the target table.
 * @param cache						The cache to use.
 */
void ItemStatsEngine::updateTopNChart(TopNChart* const chart, const Breadcrumbs& crumbs, const QSet<BufferRowIndex>& selectedBufferRows, std::function<qreal (const QList<BufferRowIndex>&)> valueFromTargetBufferRows, QMap<BufferRowIndex, qreal>& cache) const
{
	assert(chart);
	assert(valueFromTargetBufferRows);
	
	QList<QPair<BufferRowIndex, qreal>> indexValuePairs = QList<QPair<BufferRowIndex, qreal>>();
	
	// Find the desired value for every selected buffer row in the start table
	for (const BufferRowIndex& currentStartBufferIndex : selectedBufferRows) {
		qreal valueForCurrentStartIndex;
		
		// Check cache
		if (cache.contains(currentStartBufferIndex)) {
			// Cache hit
			valueForCurrentStartIndex = cache.value(currentStartBufferIndex);
		}
		else {
			// Cache miss
			const QList<BufferRowIndex> currentTargetBufferRows = crumbs.evaluateForStats({currentStartBufferIndex});
			valueForCurrentStartIndex = valueFromTargetBufferRows(currentTargetBufferRows);
			
			// Write to cache
			cache.insert(currentStartBufferIndex, valueForCurrentStartIndex);
		}
		
		if (valueForCurrentStartIndex <= 0) continue;
		
		indexValuePairs.append({currentStartBufferIndex, valueForCurrentStartIndex});
	}
	
	// Sort by value to find the N items with the highest value
	auto comparator = [](const QPair<BufferRowIndex, qreal>& pair1, const QPair<BufferRowIndex, qreal>& pair2) {
		return pair1.second > pair2.second;
	};
	std::stable_sort(indexValuePairs.begin(), indexValuePairs.end(), comparator);
	
	int numItems = std::min(chart->n, (int) indexValuePairs.size());
	QStringList itemLabels = QStringList();
	QList<qreal> itemValues = QList<qreal>();
	for (int i = 0; i < numItems; i++) {
		const QString itemLabel = getItemLabelFor(indexValuePairs.at(i).first);
		itemLabels.append(itemLabel);
		const qreal itemValue = indexValuePairs.at(i).second;
		itemValues.append(itemValue);
	}
	
	chart->updateData(itemLabels, itemValues, currentlyAllRowsSelected);
}

/**
 * Returns a label for the item at the given buffer index, for use in a top N chart.
 * 
 * @param bufferIndex	The buffer index of the item.
 * @return				A chart label for the item.
 */
QString ItemStatsEngine::getItemLabelFor(const BufferRowIndex& bufferIndex) const
{
	QString result = QString();
	
	switch (itemType) {
	case ItemTypeAscent: {
		ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(bufferIndex);
		if (peakID.isValid()) {
			result = db->peaksTable->nameColumn->getValueFor(FORCE_VALID(peakID)).toString();
		}
		if (result.isEmpty()) {
			const QDate date = db->ascentsTable->dateColumn->getValueAt(bufferIndex).toDate();
			if (date.isValid()) {
				result = date.toString("yyyy-MM-dd");
			}
		}
		break;
	}
	case ItemTypePeak: {
		result = db->peaksTable->nameColumn->getValueAt(bufferIndex).toString();
		break;
	}
	case ItemTypeTrip: {
		const QDate startDate = db->tripsTable->startDateColumn->getValueAt(bufferIndex).toDate();
		if (startDate.isValid()) {
			result = startDate.toString("yyyy-MM");
		}
		QString tripName = db->tripsTable->nameColumn->getValueAt(bufferIndex).toString();
		if (!tripName.isEmpty()) {
			if (!result.isEmpty()) result.append(" ");
			result.append(tripName);
		}
		break;
	}
	case ItemTypeHiker: {
		result = db->hikersTable->nameColumn->getValueAt(bufferIndex).toString();
		break;
	}
	case ItemTypeRegion: {
		result = db->regionsTable->nameColumn->getValueAt(bufferIndex).toString();
		break;
	}
	case ItemTypeRange: {
		result = db->rangesTable->nameColumn->getValueAt(bufferIndex).toString();
		break;
	}
	case ItemTypeCountry: {
		result = db->countriesTable->nameColumn->getValueAt(bufferIndex).toString();
		break;
	}
	default: assert(false);
	}
	
	return result;
}



/**
 * Clears both caches for the given breadcrumbs trail.
 * 
 * @param breadcrumbs	The breadcrumbs for which to clear the caches.
 */
void ItemStatsEngine::clearBreadcrumbCachesFor(const Breadcrumbs* const breadcrumbs)
{
	if (breadcrumbs == &ascentCrumbs) {
		ascentCrumbsSingleRowResultCache.clear();
		ascentCrumbsWholeSetResultCache.clear();
	}
	else if (breadcrumbs == &peakCrumbs) {
		peakCrumbsSingleRowResultCache.clear();
		peakCrumbsWholeSetResultCache.clear();
	}
}

/**
 * Clears the cache for the given chart.
 * 
 * @param chart	The chart for which to clear the cache.
 */
void ItemStatsEngine::clearChartCacheFor(Chart* const chart)
{
	if (chart == peakHeightHistChart) {
		peakHeightHistCache.clear();
	}
	else if (chart == elevGainHistChart) {
		elevGainHistCache.clear();
	}
	else if (chart == heightsScatterChart) {
		heightsScatterCache.clear();
	}
	else if (chart == topNumAscentsChart) {
		topNumAscentsCache.clear();
	}
	else if (chart == topMaxPeakHeightChart) {
		topMaxPeakHeightCache.clear();
	}
	else if (chart == topMaxElevGainChart) {
		topMaxElevGainCache.clear();
	}
	else if (chart == topElevGainSumChart) {
		topElevGainSumCache.clear();
	}
}


/**
 * Returns a map of breadcrumbs pointers and the sets of charts affected by changes to each
 * breadcrumbs trail.
 * 
 * @return	A map of breadcrumbs sets of dependent charts.
 */
QHash<const Breadcrumbs*, QSet<Chart*>> ItemStatsEngine::getBreadcrumbDependencyMap() const
{
	return {
		{&ascentCrumbs, {
			elevGainHistChart,
			heightsScatterChart,
			topNumAscentsChart,
			topMaxElevGainChart,
			topElevGainSumChart
		}},
		{&peakCrumbs, {
			peakHeightHistChart,
			topMaxPeakHeightChart
		}}
	};
}

/**
 * Returns a map of charts and the sets of columns on which they depend.
 * 
 * @return	A map of charts and the sets of columns on which they depend.
 */
QHash<Chart*, QSet<Column*>> ItemStatsEngine::getPostCrumbsUnderlyingColumnSetPerChart() const
{
	return {
		{peakHeightHistChart, {
			db->peaksTable->heightColumn
		}},
		{elevGainHistChart, {
			db->ascentsTable->elevationGainColumn
		}},
		{heightsScatterChart, {
			db->ascentsTable->dateColumn,
			db->ascentsTable->timeColumn,
			db->ascentsTable->elevationGainColumn,
			db->ascentsTable->peakIDColumn,
			db->peaksTable->heightColumn
		}},
		{topNumAscentsChart, {}},
		{topMaxPeakHeightChart, {
			db->peaksTable->heightColumn
		}},
		{topMaxElevGainChart, {
			db->ascentsTable->elevationGainColumn
		}},
		{topElevGainSumChart, {
			db->ascentsTable->elevationGainColumn
		}}
	};
}

/**
 * Returns the set of columns used for the item labels, specific to the item type.
 * 
 * @return	The set of columns used for the item labels.
 */
QSet<Column*> ItemStatsEngine::getItemLabelUnderlyingColumnSet() const
{
	switch (itemType) {
	case ItemTypeAscent:	return {db->ascentsTable->peakIDColumn, db->peaksTable->nameColumn, db->ascentsTable->dateColumn};
	case ItemTypePeak:		return {db->peaksTable->nameColumn};
	case ItemTypeTrip:		return {db->tripsTable->startDateColumn, db->tripsTable->nameColumn};
	case ItemTypeHiker:		return {db->hikersTable->nameColumn};
	case ItemTypeRegion:	return {db->regionsTable->nameColumn};
	case ItemTypeRange:		return {db->rangesTable->nameColumn};
	case ItemTypeCountry:	return {db->countriesTable->nameColumn};
	default: assert(false);
	}
	return QSet<Column*>();
}

/**
 * Returns a map of charts and the sets of columns on which they depend for the item labels.
 * 
 * @return	A map of charts and the sets of columns on which they depend for the item labels.
 */
QHash<Chart*, QSet<Column*>> ItemStatsEngine::getItemLabelUnderlyingColumnSetPerChart() const
{
	const QSet<Column*> labelColumns = getItemLabelUnderlyingColumnSet();
	return {
		{peakHeightHistChart,	{}},
		{elevGainHistChart,		{}},
		{heightsScatterChart,	{}},
		{topNumAscentsChart,	labelColumns},
		{topMaxPeakHeightChart,	labelColumns},
		{topMaxElevGainChart,	labelColumns},
		{topElevGainSumChart,	labelColumns},
	};
}

/**
 * Returns the set of columns used by this ItemStatsEngine for any of its charts.
 * 
 * Result is specific to the item type.
 * 
 * @return	The set of columns used by this ItemStatsEngine.
 */
QSet<Column*> ItemStatsEngine::getUsedColumnSet() const
{
	QSet<Column*> underlyingColumns = QSet<Column*>();
	
	underlyingColumns.unite(ascentCrumbs.getColumnSet());
	underlyingColumns.unite(peakCrumbs.getColumnSet());
	
	const QHash<Chart*, QSet<Column*>> columnsUnderCaches = getPostCrumbsUnderlyingColumnSetPerChart();
	for (const QSet<Column*>& columns : columnsUnderCaches) {
		underlyingColumns.unite(columns);
	}
	
	underlyingColumns.unite(getItemLabelUnderlyingColumnSet());
	
	return underlyingColumns;
}





/**
 * Creates a ColumnChangeListenerGeneralStatsEngine.
 *
 * @param listener			The ItemStatsEngine to notify about changes.
 * @param affectedCharts	The set of charts affected by changes to the column.
 */
ColumnChangeListenerGeneralStatsEngine::ColumnChangeListenerGeneralStatsEngine(GeneralStatsEngine* listener, const QSet<Chart*>& affectedCharts) :
	ColumnChangeListener(),
	listener(listener),
	affectedCharts(affectedCharts)
{}

/**
 * Destroys the ColumnChangeListenerGeneralStatsEngine.
 */
ColumnChangeListenerGeneralStatsEngine::~ColumnChangeListenerGeneralStatsEngine()
{}



/**
 * Notifies the listening GeneralStatsEngine that the data in the column has
 * changed.
 * 
 * @param affectedColumns	The set of columns whose contents have changed.
 */
void ColumnChangeListenerGeneralStatsEngine::columnDataChanged(QSet<const Column*> affectedColumns) const
{
	Q_UNUSED(affectedColumns);
	listener->markChartsDirty(affectedCharts);
}





/**
 * Creates a ColumnChangeListenerItemStatsEngine.
 *
 * @param listener	The ItemStatsEngine to notify about changes.
 */
ColumnChangeListenerItemStatsEngine::ColumnChangeListenerItemStatsEngine(ItemStatsEngine* listener) :
	ColumnChangeListener(),
	listener(listener)
{}

/**
 * Destroys the ColumnChangeListenerItemStatsEngine.
 */
ColumnChangeListenerItemStatsEngine::~ColumnChangeListenerItemStatsEngine()
{}



/**
 * Notifies the listening ItemStatsEngine that the data in the column has changed.
 * 
 * @param affectedColumns	The set of columns whose contents have changed.
 */
void ColumnChangeListenerItemStatsEngine::columnDataChanged(QSet<const Column*> affectedColumns) const
{
	listener->announceColumnChanges(affectedColumns);
}
