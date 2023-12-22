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
 * @param charts			The charts to add.
 * @param stretchFactors	The stretch factors to set for the layout. Can be empty, in which case all charts get a stretch factor of 1.
 */
void StatsEngine::addChartsToLayout(QBoxLayout* layout, const QList<QChartView*>& charts, QList<int> stretchFactors)
{
	assert(layout);
	
	for (QChartView* const chart : charts) {
		assert(chart);
		layout->addWidget(chart);
	}
	
	if (stretchFactors.isEmpty()) stretchFactors = QList<int>(charts.length(), 1);
	int stretchIndex = 0;
	for (const int stretchFactor : stretchFactors) {
		layout->setStretch(stretchIndex++, stretchFactor);
	}
}



/**
 * Returns a list of translated category names for the x-axis of a histogram.
 * 
 * @param increment	The value increment between categories.
 * @param max		The lower bound of the highest category.
 * @param prefix	The prefix for the translated category names (before the number).
 * @param suffix	The suffix for the translated category names (after the number).
 * @return			A list of translated category names.
 */
QStringList StatsEngine::getHistCategories(int increment, int max, QString prefix, QString suffix)
{
	QStringList categories = QStringList("&lt;" + QString::number(increment));
	for (int i = increment; i <= max; i += increment) {
		categories.append(prefix + QString::number(i) + suffix);
	}
	return categories;
}

/**
 * Assigns a class or category index to a given value for compiling a histogram.
 * 
 * @param value		The value to classify.
 * @param increment	The value increment between categories.
 * @param max		The lower bound of the highest category.
 * @return			An integer representing the class or category the given value belongs to.
 */
int StatsEngine::classifyHistValue(int value, int increment, int max)
{
	return std::min(value, max) / increment;
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
	elevGainPerYearChart	= new YearChart("Elevation gain sum per year",						"km",				false);
	numAscentsPerYearChart	= new YearChart("Number of scaled peaks per year",					"Number of peaks",	false);
	heightsScatterChart		= new YearChart("All elevation gains and peak heights over time",	"m",				true);
	
	// Set layout
	QHBoxLayout* statisticsTabUpperLayout = new QHBoxLayout();
	statisticsTabUpperLayout->setSpacing(10);
	statisticsTabUpperLayout->setObjectName("statisticsTabUpperLayout");
	QVBoxLayout* statisticsTabLayout = *statisticsTabLayoutPtr;
	assert(statisticsTabLayout);
	statisticsTabLayout->addLayout(statisticsTabUpperLayout);
	
	addChartsToLayout(statisticsTabUpperLayout, {
		elevGainPerYearChart->getChartView(),
		numAscentsPerYearChart->getChartView()
	});
	addChartsToLayout(statisticsTabLayout, {
		heightsScatterChart->getChartView()
	}, {2, 3});
}

/**
 * Computes new data for the charts in the statistics tab and updates them.
 */
void GeneralStatsEngine::updateStatsTab()
{
	assert(elevGainPerYearChart);
	assert(numAscentsPerYearChart);
	assert(heightsScatterChart);

	QLineSeries*	elevGainPerYearSeries	= Chart::createLineSeries	("Elevation gain");
	QLineSeries*	numAscentsPerYearSeries	= Chart::createLineSeries	("Number of ascended peaks");
	QScatterSeries*	peakHeightSeries		= Chart::createScatterSeries("Peak heights",				6,	QScatterSeries::MarkerShapeTriangle);
	QScatterSeries*	elevGainSeries			= Chart::createScatterSeries("Elevation gains",				6,	QScatterSeries::MarkerShapeRotatedRectangle);
	
	
	QMap<int, int> yearElevGainSums	= QMap<int, int>();
	QMap<int, int> yearNumAscents	= QMap<int, int>();
	int minYear = INT_MAX;
	int maxYear = INT_MIN;
	qreal minDate = 3000;
	qreal maxDate = 0;
	qreal elevGainPerYearMaxY = 0;
	int numAscentsPerYearMaxY = 0;
	int heightsMaxY = 0;
	for (BufferRowIndex bufferIndex = BufferRowIndex(0); bufferIndex.isValid(db->ascentsTable->getNumberOfRows()); bufferIndex++) {
		Ascent* ascent = db->getAscentAt(bufferIndex);
		
		if (ascent->dateSpecified()) {
			int year = ascent->date.year();
			if (year < minYear) minYear = year;
			if (year > maxYear) maxYear = year;
			
			yearNumAscents[year]++;
			
			qreal dateReal = (qreal) ascent->date.dayOfYear() / ascent->date.daysInYear() + ascent->date.year();
			if (dateReal < minDate) minDate = dateReal;
			if (dateReal > maxDate) maxDate = dateReal;
			
			if (ascent->elevationGainSpecified()) {
				int elevGain = ascent->elevationGain;
				elevGainSeries->append(dateReal, elevGain);
				if (elevGain > heightsMaxY) heightsMaxY = elevGain;
				yearElevGainSums[year] += elevGain;
			}
			if (ascent->peakID.isValid()) {
				const Peak* const peak = db->getPeak(FORCE_VALID(ascent->peakID));
				if (peak->heightSpecified()) {
					int peakHeight = peak->height;
					peakHeightSeries->append(dateReal, peakHeight);
					if (peakHeight > heightsMaxY) heightsMaxY = peakHeight;
				}
			}
		}
		
		delete ascent;
	}
	
	for (int year = minYear; year <= maxYear; year++) {
		int elevGainSum	= yearElevGainSums	.contains(year) ? yearElevGainSums	[year] : 0;
		int numAscents	= yearNumAscents	.contains(year) ? yearNumAscents	[year] : 0;
		qreal elevGainSumKm = (qreal) elevGainSum / 1000;
		elevGainPerYearSeries	->append(year, elevGainSumKm);
		numAscentsPerYearSeries	->append(year, numAscents);
		if (elevGainSumKm > elevGainPerYearMaxY) elevGainPerYearMaxY = elevGainSumKm;
		if (numAscents > numAscentsPerYearMaxY) numAscentsPerYearMaxY = numAscents;
	}
	
	
	elevGainPerYearChart	->updateData({elevGainPerYearSeries}, 				minYear,	maxYear,	0,	elevGainPerYearMaxY);
	numAscentsPerYearChart	->updateData({numAscentsPerYearSeries},				minYear,	maxYear,	0,	numAscentsPerYearMaxY);
	heightsScatterChart		->updateData({peakHeightSeries, elevGainSeries},	minDate,	maxDate,	0,	heightsMaxY);
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
	peakHeightHistCategoryIncrement(1000),
	peakHeightHistCategoryMax(8848),
	peakHeightHistCategories(getHistCategories(peakHeightHistCategoryIncrement, peakHeightHistCategoryMax, "", "s")),
	numPeakHeightHistCategories(peakHeightHistCategories.size()),
	elevGainHistCategoryIncrement(250),
	elevGainHistCategoryMax(1500),
	elevGainHistCategories(getHistCategories(elevGainHistCategoryIncrement, elevGainHistCategoryMax, "&ge;", "")),
	numElevGainHistCategories(elevGainHistCategories.size()),
	peakHeightHistChart(nullptr),
	elevGainHistChart(nullptr),
	heightsScatterChart(nullptr)
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
	peakHeightHistChart	= new HistogramChart("Peak height distribution", peakHeightHistCategories, "Peak heights");
	elevGainHistChart	= new HistogramChart("Elevation gain distribution", elevGainHistCategories, "Elevation gains");
	heightsScatterChart	= new YearChart("Elevation gains and peak heights over time", QString(), true);
	
	heightsScatterChart->getChartView()->setMinimumHeight(250);
	
	addChartsToLayout(statsLayout, {
		peakHeightHistChart->getChartView(),
		elevGainHistChart->getChartView(),
		heightsScatterChart->getChartView()
	});
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
	
	
	// Collect peak/ascent IDs
	
	const Breadcrumbs peakCrumbs = db->getBreadcrumbsFor(baseTable, db->peaksTable);
	const QList<BufferRowIndex> peakBufferRows = peakCrumbs.evaluateForStats(selectedBufferRows);
	
	const Breadcrumbs ascentCrumbs = db->getBreadcrumbsFor(baseTable, db->ascentsTable);
	const QList<BufferRowIndex> ascentBufferRows = ascentCrumbs.evaluateForStats(selectedBufferRows);
	
	
	// Peak height histogram
	
	QList<qreal> peakHeightHistogram = QList<qreal>(numPeakHeightHistCategories, 0);
	qreal peakHeightMaxY = 0;
	
	for (const BufferRowIndex& peakBufferRow : peakBufferRows) {
		const QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueAt(peakBufferRow);
		if (!peakHeightRaw.isValid()) continue;
		
		const int peakHeight = peakHeightRaw.toInt();
		int peakHeightClass = classifyHistValue(peakHeight, peakHeightHistCategoryIncrement, peakHeightHistCategoryMax);
		qreal newValue = ++peakHeightHistogram[peakHeightClass];
		if (newValue > peakHeightMaxY) peakHeightMaxY = newValue;
	}
	
	peakHeightHistChart->updateData(peakHeightHistogram, peakHeightMaxY);
	
	
	// Elevation gain histogram
	
	QList<qreal> elevGainHistogram = QList<qreal>(numElevGainHistCategories, 0);
	qreal elevGainMaxY = 0;
	
	for (const BufferRowIndex& ascentBufferRow : ascentBufferRows) {
		QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(ascentBufferRow);
		if (!elevGainRaw.isValid()) continue;
		
		int elevGain = elevGainRaw.toInt();
		int elevGainClass = classifyHistValue(elevGain, elevGainHistCategoryIncrement, elevGainHistCategoryMax);
		qreal newValue = ++elevGainHistogram[elevGainClass];
		if (newValue > elevGainMaxY) elevGainMaxY = newValue;
	}
	
	elevGainHistChart->updateData(elevGainHistogram, elevGainMaxY);
	
	
	// Height scatterplot
	
	QScatterSeries*	peakHeightScatterSeries	= Chart::createScatterSeries("Peak heights",	5,	QScatterSeries::MarkerShapeTriangle);
	QScatterSeries*	elevGainScatterSeries	= Chart::createScatterSeries("Elevation gains",	5,	QScatterSeries::MarkerShapeRotatedRectangle);
	qreal minDate = 3000;
	qreal maxDate = 0;
	int heightsMaxY = 0;
	
	for (const BufferRowIndex& ascentBufferIndex : ascentBufferRows) {
		Ascent* ascent = db->getAscentAt(ascentBufferIndex);
		
		if (ascent->dateSpecified()) {
			qreal dateReal = (qreal) ascent->date.dayOfYear() / ascent->date.daysInYear() + ascent->date.year();
			if (dateReal < minDate) minDate = dateReal;
			if (dateReal > maxDate) maxDate = dateReal;
			
			if (ascent->elevationGainSpecified()) {
				int elevGain = ascent->elevationGain;
				elevGainScatterSeries->append(dateReal, elevGain);
				if (elevGain > heightsMaxY) heightsMaxY = elevGain;
			}
			if (ascent->peakID.isValid()) {
				const Peak* const peak = db->getPeak(FORCE_VALID(ascent->peakID));
				if (peak->heightSpecified()) {
					int peakHeight = peak->height;
					peakHeightScatterSeries->append(dateReal, peakHeight);
					if (peakHeight > heightsMaxY) heightsMaxY = peakHeight;
				}
			}
		}
		
		delete ascent;
	}
	
	heightsScatterChart->updateData({peakHeightScatterSeries, elevGainScatterSeries}, minDate, maxDate, 0, heightsMaxY);
}
