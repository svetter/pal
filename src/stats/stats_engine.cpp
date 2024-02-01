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
	db(db)
{}

/**
 * Destroys the StatsEngine.
 */
StatsEngine::~StatsEngine()
{}



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
	elevGainPerYearChart(nullptr),
	numAscentsPerYearChart(nullptr),
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
 * Creates and initializes the charts for the statistics tab and sets up the layout.
 */
void GeneralStatsEngine::setupStatsTab()
{
	elevGainPerYearChart	= new YearBarChart		(tr("Elevation gain sum per year"),						tr("km"));
	numAscentsPerYearChart	= new YearBarChart		(tr("Number of ascents per year"),						tr("Number of ascents"));
	heightsScatterChart		= new TimeScatterChart	(tr("All elevation gains and peak heights over time"),	tr("m"));
	
	// Set layout
	QHBoxLayout* statisticsTabUpperLayout = new QHBoxLayout();
	statisticsTabUpperLayout->setSpacing(10);
	statisticsTabUpperLayout->setObjectName("statisticsTabUpperLayout");
	QVBoxLayout* statisticsTabLayout = *statisticsTabLayoutPtr;
	assert(statisticsTabLayout);
	statisticsTabLayout->addLayout(statisticsTabUpperLayout);
	
	addChartsToLayout(statisticsTabUpperLayout, {
		elevGainPerYearChart,
		numAscentsPerYearChart
	});
	addChartsToLayout(statisticsTabLayout, {
		heightsScatterChart
	}, {2, 3});
}

/**
 * Resets the charts in the statistics tab.
 */
void GeneralStatsEngine::resetStatsTab()
{
	assert(elevGainPerYearChart);
	assert(numAscentsPerYearChart);
	assert(heightsScatterChart);
	
	elevGainPerYearChart	->reset();
	numAscentsPerYearChart	->reset();
	heightsScatterChart		->reset();
}

/**
 * Computes new data for the charts in the statistics tab and updates them.
 */
void GeneralStatsEngine::updateStatsTab()
{
	assert(elevGainPerYearChart);
	assert(numAscentsPerYearChart);
	assert(heightsScatterChart);
	
	QList<qreal>	elevGainPerYearSeries	= QList<qreal>();
	QList<qreal>	numAscentsPerYearSeries	= QList<qreal>();
	DateScatterSeries	elevGainSeries		= DateScatterSeries(tr("Elevation gains"),	6,	QScatterSeries::MarkerShapeRotatedRectangle);
	DateScatterSeries	peakHeightSeries	= DateScatterSeries(tr("Peak heights"),		6,	QScatterSeries::MarkerShapeTriangle);
	
	
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
		
		if (date < minDate || !minDate.isValid()) minDate = date;
		if (date > maxDate || !maxDate.isValid()) maxDate = date;
		
		QTime time = db->ascentsTable->timeColumn->getValueAt(bufferIndex).toTime();
		if (!time.isValid()) time = QTime(12, 0);
		const QDateTime dateTime = QDateTime(date, time);
		
		const int year = date.year();
		yearNumAscents[year]++;
		
		const QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(bufferIndex);
		if (elevGainRaw.isValid()) {
			int elevGain = elevGainRaw.toInt();
			elevGainSeries.data.append({dateTime, elevGain});
			if (elevGain > heightsMaxY) heightsMaxY = elevGain;
			yearElevGainSums[year] += elevGain;
		}
		
		const ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(bufferIndex);
		if (peakID.isValid()) {
			QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueFor(FORCE_VALID(peakID));
			if (peakHeightRaw.isValid()) {
				int peakHeight = peakHeightRaw.toInt();
				peakHeightSeries.data.append({dateTime, peakHeight});
				if (peakHeight > heightsMaxY) heightsMaxY = peakHeight;
			}
		}
	}
	
	int minYear = minDate.year();
	int maxYear = maxDate.year();
	
	for (int year = minYear; year <= maxYear; year++) {
		int elevGainSum	= yearElevGainSums	.contains(year) ? yearElevGainSums	[year] : 0;
		int numAscents	= yearNumAscents	.contains(year) ? yearNumAscents	[year] : 0;
		qreal elevGainSumKm = (qreal) elevGainSum / 1000;
		elevGainPerYearSeries	.append(elevGainSumKm);
		numAscentsPerYearSeries	.append(numAscents);
		if (elevGainSumKm > elevGainPerYearMaxY) elevGainPerYearMaxY = elevGainSumKm;
		if (numAscents > numAscentsPerYearMaxY) numAscentsPerYearMaxY = numAscents;
	}
	
	
	elevGainPerYearChart	->updateData(elevGainPerYearSeries, 	minYear,	maxYear,	elevGainPerYearMaxY);
	numAscentsPerYearChart	->updateData(numAscentsPerYearSeries,	minYear,	maxYear,	numAscentsPerYearMaxY);
	
	const QList<DateScatterSeries*> heightsScatterSeries = {&elevGainSeries, &peakHeightSeries};
	heightsScatterChart->updateData(heightsScatterSeries, minDate, maxDate, heightsMaxY);
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
	itemType(itemType),
	baseTable(baseTable),
	statsLayout(statsLayout),
	peakHeightHistChart		(nullptr),
	elevGainHistChart		(nullptr),
	heightsScatterChart		(nullptr),
	topNumAscentsChart		(nullptr),
	topMaxPeakHeightChart	(nullptr),
	topMaxElevGainChart		(nullptr),
	topElevGainSumChart		(nullptr),
	ascentCrumbs	(db->getBreadcrumbsFor(baseTable, db->ascentsTable)),
	peakCrumbs		(db->getBreadcrumbsFor(baseTable, db->peaksTable)),
	ascentCrumbsResultCache	(QMap<BufferRowIndex, QList<BufferRowIndex>>()),
	peakCrumbsResultCache	(QMap<BufferRowIndex, QList<BufferRowIndex>>()),
	peakHeightHistCache		(QMap<BufferRowIndex, int>()),
	elevGainHistCache		(QMap<BufferRowIndex, int>()),
	heightsScatterCache		(QMap<BufferRowIndex, QPair<QDateTime, QList<qreal>>>()),
	topNumAscentsCache		(QMap<BufferRowIndex, qreal>()),
	topMaxPeakHeightCache	(QMap<BufferRowIndex, qreal>()),
	topMaxElevGainCache		(QMap<BufferRowIndex, qreal>()),
	topElevGainSumCache		(QMap<BufferRowIndex, qreal>())
{
	assert(statsLayout);
}

/**
 * Destroys the ItemStatsEngine.
 */
ItemStatsEngine::~ItemStatsEngine()
{
	delete peakHeightHistChart;
	delete elevGainHistChart;
}



/**
 * Creates and initializes the charts for the statistics panel.
 */
void ItemStatsEngine::setupStatsPanel()
{
	// Peak height histogram
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
		itemType != ItemTypeAscent ? topNumAscentsChart : nullptr,
		topMaxPeakHeightChart,
		topMaxElevGainChart,
		itemType != ItemTypeAscent ? topElevGainSumChart : nullptr
	});
}

/**
 * Resets the charts in the statistics panel.
 */
void ItemStatsEngine::resetStatsPanel()
{
	assert(peakHeightHistChart);
	assert(elevGainHistChart);
	assert(heightsScatterChart);
	assert((topNumAscentsChart != nullptr) != (itemType == ItemTypeAscent));
	assert(topMaxPeakHeightChart);
	assert(topMaxElevGainChart);
	assert((topElevGainSumChart != nullptr) != (itemType == ItemTypeAscent));
	
	peakHeightHistChart->reset();
	elevGainHistChart->reset();
	heightsScatterChart->reset();
	if (topNumAscentsChart) topNumAscentsChart->reset();
	topMaxPeakHeightChart->reset();
	topMaxElevGainChart->reset();
	if (topElevGainSumChart) topElevGainSumChart->reset();
}



/**
 * Resets all caches for breadcrumb results and chart data.
 * 
 * To be called when the underlying data has changed.
 */
void ItemStatsEngine::resetCaches()
{
	ascentCrumbsResultCache	.clear();
	peakCrumbsResultCache	.clear();
	peakHeightHistCache		.clear();
	elevGainHistCache		.clear();
	heightsScatterCache		.clear();
	topNumAscentsCache		.clear();
	topMaxPeakHeightCache	.clear();
	topMaxElevGainCache		.clear();
	topElevGainSumCache		.clear();
}



/**
 * Computes new data for the charts in the statistics panel and updates them.
 * 
 * @param selectedBufferRows	The buffer rows of all items currently selected in the UI table.
 */
void ItemStatsEngine::updateStatsPanel(const QSet<BufferRowIndex>& selectedBufferRows)
{
	assert(peakHeightHistChart);
	assert(elevGainHistChart);
	assert(heightsScatterChart);
	assert((topNumAscentsChart != nullptr) != (itemType == ItemTypeAscent));
	assert(topMaxPeakHeightChart);
	assert(topMaxElevGainChart);
	assert((topElevGainSumChart != nullptr) != (itemType == ItemTypeAscent));
	
	
	// Collect peak/ascent IDs
	
	QList<BufferRowIndex> ascentBufferRows	= evaluateCrumbsCached(ascentCrumbs,	selectedBufferRows, ascentCrumbsResultCache);
	QList<BufferRowIndex> peakBufferRows	= evaluateCrumbsCached(peakCrumbs,		selectedBufferRows, peakCrumbsResultCache);
	
	
	// Peak height histogram
	
	if (peakHeightHistChart) {
		auto peakHeightClassFromPeakBufferRow = [this](const BufferRowIndex& peakBufferRow) {
			const QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueAt(peakBufferRow);
			if (!peakHeightRaw.isValid()) return -1;
			
			const int peakHeight = peakHeightRaw.toInt();
			return peakHeightHistChart->classifyValue(peakHeight);
		};
		
		updateHistogramChart(peakHeightHistChart, peakBufferRows, peakHeightClassFromPeakBufferRow, peakHeightHistCache);
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
	}
	
	
	// Top N with most ascents chart
	
	if (topNumAscentsChart) {
		assert(itemType != ItemTypeAscent);
		
		auto numAscentsFromAscentBufferRows = [](const QList<BufferRowIndex>& ascentBufferRows) {
			return ascentBufferRows.size();
		};
		
		updateTopNChart(topNumAscentsChart, ascentCrumbs, selectedBufferRows, numAscentsFromAscentBufferRows, topNumAscentsCache);
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
		
		updateTopNChart(topMaxPeakHeightChart, peakCrumbs, selectedBufferRows, maxPeakHeightFromPeakBufferRows, topMaxPeakHeightCache);
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
		
		updateTopNChart(topMaxElevGainChart, ascentCrumbs, selectedBufferRows, maxElevGainFromAscentBufferRows, topMaxElevGainCache);
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
		
		updateTopNChart(topElevGainSumChart, ascentCrumbs, selectedBufferRows, elevGainSumFromAscentBufferRows, topElevGainSumCache);
	}
}


/**
 * Returns the evaluation of the given breadcrumbs for the given selected buffer rows, using or
 * updating the given cache.
 * 
 * @param crumbs				The breadcrumbs to evaluate.
 * @param selectedBufferRows	The buffer rows of all items currently selected in the UI table.
 * @param crumbsResultCache		The cache to use.
 * @return						The evaluation of the given breadcrumbs for the given selected buffer rows.
 */
QList<BufferRowIndex> ItemStatsEngine::evaluateCrumbsCached(const Breadcrumbs& crumbs, const QSet<BufferRowIndex>& selectedBufferRows, QMap<BufferRowIndex, QList<BufferRowIndex>>& crumbsResultCache) const
{
	QList<BufferRowIndex> targetBufferRows = QList<BufferRowIndex>();
	
	for (const BufferRowIndex& currentBufferRow : selectedBufferRows) {
		QList<BufferRowIndex> newTargetBufferRows;
		
		// Check cache
		if (crumbsResultCache.contains(currentBufferRow)) {
			// Cache hit
			newTargetBufferRows = crumbsResultCache.value(currentBufferRow);
		}
		else {
			// Cache miss
			newTargetBufferRows = crumbs.evaluateForStats({currentBufferRow});
			
			// Write to cache
			crumbsResultCache.insert(currentBufferRow, newTargetBufferRows);
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
	
	chart->updateData(histogramData, maxY);
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
	
	chart->updateData(allSeries, minDate, maxDate, maxY);
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
	
	chart->updateData(itemLabels, itemValues);
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
		const QDate date = db->ascentsTable->dateColumn->getValueAt(bufferIndex).toDate();
		if (date.isValid()) {
			result = date.toString("yyyy-MM-dd");
		}
		QString peakName = QString();
		ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(bufferIndex);
		if (peakID.isValid()) {
			peakName = db->peaksTable->nameColumn->getValueFor(FORCE_VALID(peakID)).toString();
		}
		if (!peakName.isEmpty()) {
			if (!result.isEmpty()) result.append(" ");
			result.append(peakName);
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
 * Returns the set of columns used by this ItemStatsEngine for any of its charts.
 * 
 * Result is specific to the item type.
 * 
 * @return	The set of columns used by this ItemStatsEngine.
 */
QSet<Column* const> ItemStatsEngine::getUsedColumnSet() const
{
	QSet<Column* const> underlyingColumns = QSet<Column* const>();
	
	underlyingColumns.unite(ascentCrumbs.getColumnSet());
	underlyingColumns.unite(peakCrumbs.getColumnSet());
	
	underlyingColumns.insert(db->ascentsTable->dateColumn);
	underlyingColumns.insert(db->ascentsTable->timeColumn);
	underlyingColumns.insert(db->ascentsTable->elevationGainColumn);
	underlyingColumns.insert(db->ascentsTable->peakIDColumn);
	underlyingColumns.insert(db->peaksTable->heightColumn);
	
	switch (itemType) {
	case ItemTypeAscent: {
		underlyingColumns.insert(db->ascentsTable->dateColumn);
		underlyingColumns.insert(db->ascentsTable->peakIDColumn);
		underlyingColumns.insert(db->peaksTable->nameColumn);
		break;
	}
	case ItemTypePeak: {
		underlyingColumns.insert(db->peaksTable->nameColumn);
		break;
	}
	case ItemTypeTrip: {
		underlyingColumns.insert(db->tripsTable->startDateColumn);
		underlyingColumns.insert(db->tripsTable->nameColumn);
		break;
	}
	case ItemTypeHiker: {
		underlyingColumns.insert(db->hikersTable->nameColumn);
		break;
	}
	case ItemTypeRegion: {
		underlyingColumns.insert(db->regionsTable->nameColumn);
		break;
	}
	case ItemTypeRange: {
		underlyingColumns.insert(db->rangesTable->nameColumn);
		break;
	}
	case ItemTypeCountry: {
		underlyingColumns.insert(db->countriesTable->nameColumn);
		break;
	}
	default: assert(false);
	}
	
	return underlyingColumns;
}
