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
		QVariant dateRaw = db->ascentsTable->dateColumn->getValueAt(bufferIndex);
		if (dateRaw.isValid()) {
			QDate date = dateRaw.toDate();
			int year = date.year();
			if (year < minYear) minYear = year;
			if (year > maxYear) maxYear = year;
			
			yearNumAscents[year]++;
			
			qreal dateReal = (qreal) date.dayOfYear() / date.daysInYear() + date.year();
			if (dateReal < minDate) minDate = dateReal;
			if (dateReal > maxDate) maxDate = dateReal;
			
			QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(bufferIndex);
			if (elevGainRaw.isValid()) {
				int elevGain = elevGainRaw.toInt();
				elevGainSeries->append(dateReal, elevGain);
				if (elevGain > heightsMaxY) heightsMaxY = elevGain;
				yearElevGainSums[year] += elevGain;
			}
			ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(bufferIndex);
			if (peakID.isValid()) {
				QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueFor(FORCE_VALID(peakID));
				if (peakHeightRaw.isValid()) {
					int peakHeight = peakHeightRaw.toInt();
					peakHeightSeries->append(dateReal, peakHeight);
					if (peakHeight > heightsMaxY) heightsMaxY = peakHeight;
				}
			}
		}
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
	heightsScatterChart(nullptr),
	topTenNumAscentsChart(nullptr),
	topTenMaxPeakHeightChart(nullptr),
	topTenMaxElevGainChart(nullptr),
	topTenElevGainSumChart(nullptr)
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
	peakHeightHistChart	= new HistogramChart("Peak height distribution", peakHeightHistCategories);
	elevGainHistChart	= new HistogramChart("Elevation gain distribution", elevGainHistCategories);
	
	heightsScatterChart	= new YearChart("Elevation gains and peak heights over time", QString(), true);
	
	if (itemType != ItemTypeAscent) {
		topTenNumAscentsChart	= new TopNChart(10, "Most ascents");
	}
	topTenMaxPeakHeightChart	= new TopNChart(10, "Highest peak");
	topTenMaxElevGainChart		= new TopNChart(10, "Highest elevation gain");
	if (itemType != ItemTypeAscent) {
		topTenElevGainSumChart	= new TopNChart(10, "Highest elevation gain sum [km]");
	}
	
	heightsScatterChart->getChartView()->setMinimumHeight(250);
	
	addChartsToLayout(statsLayout, {
		peakHeightHistChart,
		elevGainHistChart,
		heightsScatterChart,
		itemType != ItemTypeAscent ? topTenNumAscentsChart : nullptr,
		topTenMaxPeakHeightChart,
		topTenMaxElevGainChart,
		itemType != ItemTypeAscent ? topTenElevGainSumChart : nullptr
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
	assert(topTenNumAscentsChart || itemType == ItemTypeAscent);
	assert(topTenMaxPeakHeightChart);
	assert(topTenMaxElevGainChart);
	assert(topTenElevGainSumChart || itemType == ItemTypeAscent);
	
	peakHeightHistChart->reset();
	elevGainHistChart->reset();
	heightsScatterChart->reset();
	if (topTenNumAscentsChart) topTenNumAscentsChart->reset();
	topTenMaxPeakHeightChart->reset();
	topTenMaxElevGainChart->reset();
	if (topTenElevGainSumChart) topTenElevGainSumChart->reset();
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
	assert(topTenNumAscentsChart || itemType == ItemTypeAscent);
	assert(topTenMaxPeakHeightChart);
	assert(topTenMaxElevGainChart);
	assert(topTenElevGainSumChart || itemType == ItemTypeAscent);
	
	
	// Collect peak/ascent IDs
	
	const Breadcrumbs ascentCrumbs = db->getBreadcrumbsFor(baseTable, db->ascentsTable);
	const QList<BufferRowIndex> ascentBufferRows = ascentCrumbs.evaluateForStats(selectedBufferRows);
	
	const Breadcrumbs peakCrumbs = db->getBreadcrumbsFor(baseTable, db->peaksTable);
	const QList<BufferRowIndex> peakBufferRows = peakCrumbs.evaluateForStats(selectedBufferRows);
	
	
	// Peak height histogram
	
	if (peakHeightHistChart) {
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
	}
	
	
	// Elevation gain histogram
	
	if (elevGainHistChart) {
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
	}
	
	
	// Heights scatterplot
	
	if (heightsScatterChart) {
		QScatterSeries*	peakHeightScatterSeries	= Chart::createScatterSeries("Peak heights",	5,	QScatterSeries::MarkerShapeTriangle);
		QScatterSeries*	elevGainScatterSeries	= Chart::createScatterSeries("Elevation gains",	5,	QScatterSeries::MarkerShapeRotatedRectangle);
		qreal minDate = 3000;
		qreal maxDate = 0;
		int heightsMaxY = 0;
		
		for (const BufferRowIndex& ascentBufferIndex : ascentBufferRows) {
			QVariant dateRaw = db->ascentsTable->dateColumn->getValueAt(ascentBufferIndex);
			if (dateRaw.isValid()) {
				QDate date = dateRaw.toDate();
				qreal dateReal = (qreal) date.dayOfYear() / date.daysInYear() + date.year();
				if (dateReal < minDate) minDate = dateReal;
				if (dateReal > maxDate) maxDate = dateReal;
				
				QVariant elevGainRaw = db->ascentsTable->elevationGainColumn->getValueAt(ascentBufferIndex);
				if (elevGainRaw.isValid()) {
					int elevGain = elevGainRaw.toInt();
					elevGainScatterSeries->append(dateReal, elevGain);
					if (elevGain > heightsMaxY) heightsMaxY = elevGain;
				}
				ItemID peakID = db->ascentsTable->peakIDColumn->getValueAt(ascentBufferIndex);
				if (peakID.isValid()) {
					QVariant peakHeightRaw = db->peaksTable->heightColumn->getValueFor(FORCE_VALID(peakID));
					if (peakHeightRaw.isValid()) {
						int peakHeight = peakHeightRaw.toInt();
						peakHeightScatterSeries->append(dateReal, peakHeight);
						if (peakHeight > heightsMaxY) heightsMaxY = peakHeight;
					}
				}
			}
		}
		
		heightsScatterChart->updateData({peakHeightScatterSeries, elevGainScatterSeries}, minDate, maxDate, 0, heightsMaxY);
	}
	
	
	// Top 10 with most ascents chart
	
	if (topTenNumAscentsChart) {
		assert(itemType != ItemTypeAscent);
		
		auto numAscentsFromAscentBufferRows = [](const QList<BufferRowIndex>& ascentBufferRows) {
			return ascentBufferRows.size();
		};
		
		updateTopNChart(topTenNumAscentsChart, ascentCrumbs, selectedBufferRows, numAscentsFromAscentBufferRows);
	}
	
	
	// Top 10 with highest peaks chart
	
	if (topTenMaxPeakHeightChart) {
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
		
		updateTopNChart(topTenMaxPeakHeightChart, peakCrumbs, selectedBufferRows, maxPeakHeightFromPeakBufferRows);
	}
	
	
	// Top 10 with highest single elevation gain chart
	
	if (topTenMaxElevGainChart) {
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
		
		updateTopNChart(topTenMaxElevGainChart, ascentCrumbs, selectedBufferRows, maxElevGainFromAscentBufferRows);
	}
	
	
	// Top 10 with highest elevation gain sum chart
	
	if (topTenElevGainSumChart) {
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
		
		updateTopNChart(topTenElevGainSumChart, ascentCrumbs, selectedBufferRows, elevGainSumFromAscentBufferRows);
	}
}



/**
 * Compiles data for an update of a top N chart and updates it.
 * 
 * @param chart						The chart to update.
 * @param crumbs					The breadcrumbs leading to the target table containing the data to be compared.
 * @param selectedBufferRows		The buffer rows of all items currently selected in the UI table.
 * @param valueFromTargetBufferRows	A function which returns a chart value for a given list of buffer rows in the target table.
 */
void ItemStatsEngine::updateTopNChart(TopNChart* const chart, const Breadcrumbs& crumbs, const QSet<BufferRowIndex>& selectedBufferRows, std::function<qreal (const QList<BufferRowIndex>&)> valueFromTargetBufferRows) const
{
	assert(chart);
	assert(valueFromTargetBufferRows);
	
	QList<QPair<BufferRowIndex, qreal>> indexValuePairs = QList<QPair<BufferRowIndex, qreal>>();
	
	// Find the desired value for every selected buffer row in the start table
	for (const BufferRowIndex& currentStartBufferIndex : selectedBufferRows) {
		const QList<BufferRowIndex> currentTargetBufferRows = crumbs.evaluateForStats({currentStartBufferIndex});
		qreal valueForCurrentStartIndex = valueFromTargetBufferRows(currentTargetBufferRows);
		
		if (valueForCurrentStartIndex < 1) continue;
		
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
		QVariant dateRaw = db->ascentsTable->dateColumn->getValueAt(bufferIndex);
		if (dateRaw.isValid()) {
			QDate date = dateRaw.toDate();
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
		QVariant startDateRaw = db->tripsTable->startDateColumn->getValueAt(bufferIndex);
		if (startDateRaw.isValid()) {
			QDate startDate = startDateRaw.toDate();
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
