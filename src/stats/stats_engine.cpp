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

#include "src/comp_tables/breadcrumbs.h"



StatsEngine::StatsEngine(Database* db) :
	db(db)
{}

StatsEngine::~StatsEngine()
{}



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





GeneralStatsEngine::GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr) :
	StatsEngine(db),
	statisticsTabLayoutPtr(statisticsTabLayoutPtr),
	elevGainPerYearChart(nullptr),
	numAscentsPerYearChart(nullptr),
	heightsScatterChart(nullptr)
{
	assert(statisticsTabLayoutPtr);
}

GeneralStatsEngine::~GeneralStatsEngine()
{
	delete elevGainPerYearChart;
	delete numAscentsPerYearChart;
	delete heightsScatterChart;
}



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
	
	addChartsToLayout(statisticsTabUpperLayout, {elevGainPerYearChart->getChartView(), numAscentsPerYearChart->getChartView()});
	addChartsToLayout(statisticsTabLayout, {heightsScatterChart->getChartView()}, {2, 3});
}

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





ItemStatsEngine::ItemStatsEngine(Database* db, QVBoxLayout* statsFrameLayout) :
	StatsEngine(db),
	statsFrameLayout(statsFrameLayout),
	peakHeightHistChart(nullptr),
	elevGainHistChart(nullptr)
{
	assert(statsFrameLayout);
}

ItemStatsEngine::~ItemStatsEngine()
{
	delete peakHeightHistChart;
	delete elevGainHistChart;
}



void ItemStatsEngine::setupStatsPanel()
{
	peakHeightHistChart	= new HistogramChart("Peak height distribution",	{"&lt;1000", "1000s", "2000s", "3000s", "4000s", "5000s", "6000s", "7000s", "8000s"},	"Peak heights");
	elevGainHistChart	= new HistogramChart("Elevation gain distribution",	{"&lt;250", "&ge;250", "&ge;500", "&ge;750", "&ge;1000", "&ge;1250", "&ge;1500"},		"Elevation gains");
	
	statsFrameLayout->addWidget(peakHeightHistChart->getChartView());
	statsFrameLayout->addWidget(elevGainHistChart->getChartView());
}

void ItemStatsEngine::updateStatsPanel(const QSet<BufferRowIndex>& selectedBufferRows)
{
	assert(peakHeightHistChart);
	assert(elevGainHistChart);


	// Peak height histogram
	
	const Breadcrumbs peakHeightCrumbs = Breadcrumbs({
		{db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn},
		{db->ascentsTable->peakIDColumn,	db->peaksTable->primaryKeyColumn}
	});
	QList<BufferRowIndex> peakBufferRows = peakHeightCrumbs.evaluateForStats(selectedBufferRows);
	
	QList<int> peakHeights = QList<int>();
	for (const BufferRowIndex& peakBufferRow : peakBufferRows) {
		QVariant peakHeight = db->peaksTable->heightColumn->getValueAt(peakBufferRow);
		if (!peakHeight.isValid()) continue;
		peakHeights.append(peakHeight.toInt());
	}
	
	int numPeakHeightCategories = 8848 / 1000 + 1;
	QList<qreal> peakHeightHistogram = QList<qreal>(numPeakHeightCategories, 0);
	qreal peakHeightMaxY = 0;
	for (const int peakHeight : peakHeights) {
		qreal newValue = ++peakHeightHistogram[peakHeight / 1000];
		if (newValue > peakHeightMaxY) peakHeightMaxY = newValue;
	}
	
	peakHeightHistChart->updateData(peakHeightHistogram, peakHeightMaxY);
	
	
	// Elevation gain histogram
	
	const Breadcrumbs elevGainCrumbs = Breadcrumbs({
		{db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn}
	});
	QList<BufferRowIndex> ascentBufferRows = elevGainCrumbs.evaluateForStats(selectedBufferRows);
	
	QList<int> elevGains = QList<int>();
	for (const BufferRowIndex& ascentBufferRow : ascentBufferRows) {
		QVariant elevGain = db->ascentsTable->elevationGainColumn->getValueAt(ascentBufferRow);
		if (!elevGain.isValid()) continue;
		elevGains.append(elevGain.toInt());
	}
	
	int numElevGainCategories = 1500 / 250 + 1;
	QList<qreal> elevGainHistogram = QList<qreal>(numElevGainCategories, 0);
	qreal elevGainMaxY = 0;
	for (const int elevGain : elevGains) {
		int elevGainClass = std::min(elevGain, 1500) / 250;
		qreal newValue = ++elevGainHistogram[elevGainClass];
		if (newValue > elevGainMaxY) elevGainMaxY = newValue;
	}
	
	elevGainHistChart->updateData(elevGainHistogram, elevGainMaxY);
}
