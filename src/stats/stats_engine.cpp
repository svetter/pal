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

#include <QGraphicsLayout>
#include <QHorizontalBarSeries>
#include <QBarCategoryAxis>



StatsEngine::StatsEngine(Database* db) :
	db(db)
{}



QChart* StatsEngine::createChart(const QString& title, bool displayLegend)
{
	QChart* chart = new QChart();
	chart->setMargins(QMargins(0, 0, 0, 0));
	chart->layout()->setContentsMargins(0, 0, 0, 0);
	chart->setBackgroundRoundness(0);
	chart->setTitle(title);
	QFont titleFont = chart->titleFont();
	titleFont.setPointSize(titleFont.pointSize() * 1.2);
	titleFont.setBold(true);
	chart->setTitleFont(titleFont);
	if (!displayLegend) chart->legend()->hide();
	return chart;
}

QValueAxis* StatsEngine::createValueXAxis(QChart* chart, const QString& title)
{
	assert(chart);
	
	QValueAxis* xAxis = new QValueAxis();
	xAxis->setLabelFormat("%.0f");
	xAxis->setTickType(QValueAxis::TicksDynamic);
	if (!title.isEmpty()) xAxis->setTitleText(title);
	chart->addAxis(xAxis, Qt::AlignBottom);
	return xAxis;
}

QBarCategoryAxis* StatsEngine::createBarCategoryXAxis(QChart* chart, const QStringList& categories, const Qt::AlignmentFlag alignment)
{
	QBarCategoryAxis* xAxis = new QBarCategoryAxis();
	xAxis->setCategories(categories);
	chart->addAxis(xAxis, alignment);
	return xAxis;
}

QValueAxis* StatsEngine::createValueYAxis(QChart* chart, const QString& title, const Qt::AlignmentFlag alignment)
{
	assert(chart);
	
	QValueAxis* yAxis = new QValueAxis();
	yAxis->setLabelFormat("%.0f");
	yAxis->setTickType(QValueAxis::TicksDynamic);
	if (!title.isEmpty()) yAxis->setTitleText(title);
	chart->addAxis(yAxis, alignment);
	return yAxis;
}

QChartView* StatsEngine::createChartView(QChart* chart, int minimumHeight)
{
	assert(chart);
	
	QChartView* chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);
	if (minimumHeight >= 0) chartView->setMinimumHeight(minimumHeight);
	return chartView;
}


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


QHorizontalBarSeries* StatsEngine::createHorizontalBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis)
{
	QHorizontalBarSeries* series = new QHorizontalBarSeries();
	chart->addSeries(series);
	series->attachAxis(xAxis);
	series->attachAxis(yAxis);
	return series;
}

QBarSet* StatsEngine::createBarSet(const QString& name, QAbstractBarSeries* series)
{
	QBarSet* barSet = new QBarSet(name);
	series->append(barSet);
	return barSet;
}



QLineSeries* StatsEngine::createLineSeries(const QString& name)
{
	QLineSeries* series = new QLineSeries();
	series->setName(name);
	return series;
}

QScatterSeries* StatsEngine::createScatterSeries(const QString& name, int markerSize, QScatterSeries::MarkerShape marker)
{
	QScatterSeries* series = new QScatterSeries();
	series->setName(name);
	if (markerSize >= 0)	series->setMarkerSize(markerSize);
	if (marker >= 0)		series->setMarkerShape(QScatterSeries::MarkerShapeTriangle);
	series->setPen(QPen(Qt::transparent));
	return series;
}


void StatsEngine::updateSeriesForChartWithYearXAxis(QChart* chart, QValueAxis* xAxis, QValueAxis* yAxis, const QList<QXYSeries*>& newSeries, qreal minYear, qreal maxYear, qreal minY, qreal maxY, bool bufferXAxisRange)
{
	chart->removeAllSeries();
	for (QXYSeries* const series : newSeries) {
		chart->addSeries(series);
		series->attachAxis(xAxis);
		series->attachAxis(yAxis);
	}
	
	adjustAxis(xAxis,	minYear,	maxYear,	chart->plotArea().width(),	bufferXAxisRange ? 0.01 : 0);
	adjustAxis(yAxis,	minY,		maxY,		chart->plotArea().height(),	0.05);
}


void StatsEngine::adjustAxis(QValueAxis* axis, qreal minValue, qreal maxValue, int chartSize, qreal rangeBufferFactor)
{
	assert(axis);
	assert(minValue <= maxValue);
	if (chartSize <= 0) return;
	
	// Find appropriate tick spacings
	qreal range = maxValue - minValue;
	qreal unroundedInterval = range / chartSize * pixelsPerTick;
	int log10times3 = (int) std::round(std::log10(unroundedInterval) * 3);
	int exp = log10times3 / 3;
	int interval = std::pow(10, exp);
	int minorCount;
	switch (log10times3 % 3) {
	case 0:						minorCount = 1;	break;
	case 1:		interval *= 2;	minorCount = 1;	break;
	case 2:		interval *= 5;	minorCount = 4;	break;
	default:	interval = 1;	minorCount = 0;	break;
	}
	if (interval < 2) minorCount = 0;
	int anchor = (int) minValue / interval * interval;
	
	// Add buffer on the beginning and end of the range
	qreal rangeBuffer = range * rangeBufferFactor;
	qreal rangeMin = minValue == 0 ? 0 : (minValue - rangeBuffer);
	qreal rangeMax = maxValue == 0 ? 0 : (maxValue + rangeBuffer);
	
	axis->setRange(rangeMin, rangeMax);
	axis->setTickAnchor(anchor);
	axis->setTickInterval(interval);
	axis->setMinorTickCount(minorCount);
}





GeneralStatsEngine::GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr) :
	StatsEngine(db),
	statisticsTabLayoutPtr(statisticsTabLayoutPtr),
	elevGainPerYearChart(nullptr),
	elevGainPerYearXAxis(nullptr),
	elevGainPerYearYAxis(nullptr),
	elevGainPerYearChartView(nullptr),
	numAscentsPerYearChart(nullptr),
	numAscentsPerYearXAxis(nullptr),
	numAscentsPerYearYAxis(nullptr),
	numAscentsPerYearChartView(nullptr),
	heightsScatterChart(nullptr),
	heightsScatterXAxis(nullptr),
	heightsScatterYAxis(nullptr),
	heightsScatterChartView(nullptr)
{
	assert(statisticsTabLayoutPtr);
}



void GeneralStatsEngine::setupStatsTab()
{
	elevGainPerYearChart		= createChart("Elevation gain sum per year", false);
	elevGainPerYearXAxis		= createValueXAxis(elevGainPerYearChart);
	elevGainPerYearYAxis		= createValueYAxis(elevGainPerYearChart, "km");
	elevGainPerYearChartView	= createChartView(elevGainPerYearChart);
	
	numAscentsPerYearChart		= createChart("Number of scaled peaks per year", false);
	numAscentsPerYearXAxis		= createValueXAxis(numAscentsPerYearChart);
	numAscentsPerYearYAxis		= createValueYAxis(numAscentsPerYearChart, "Number of peaks");
	numAscentsPerYearChartView	= createChartView(numAscentsPerYearChart);
	
	heightsScatterChart			= createChart("All elevation gains and peak heights over time", true);
	heightsScatterXAxis			= createValueXAxis(heightsScatterChart);
	heightsScatterYAxis			= createValueYAxis(heightsScatterChart, "m");
	heightsScatterChartView		= createChartView(heightsScatterChart);
	
	// Set layout
	QHBoxLayout* statisticsTabUpperLayout = new QHBoxLayout();
	statisticsTabUpperLayout->setSpacing(10);
	statisticsTabUpperLayout->setObjectName("statisticsTabUpperLayout");
	QVBoxLayout* statisticsTabLayout = *statisticsTabLayoutPtr;
	assert(statisticsTabLayout);
	statisticsTabLayout->addLayout(statisticsTabUpperLayout);
	
	addChartsToLayout(statisticsTabUpperLayout,	{elevGainPerYearChartView, numAscentsPerYearChartView});
	addChartsToLayout(statisticsTabLayout,		{heightsScatterChartView}, {2, 3});
}

void GeneralStatsEngine::updateStatsTab()
{
	QLineSeries*	elevGainPerYearSeries	= createLineSeries		("Elevation gain");
	QLineSeries*	numAscentsPerYearSeries	= createLineSeries		("Number of ascended peaks");
	QScatterSeries*	peakHeightSeries		= createScatterSeries	("Peak heights",	6,	QScatterSeries::MarkerShapeTriangle);
	QScatterSeries*	elevGainSeries			= createScatterSeries	("Elevation gains",	6,	QScatterSeries::MarkerShapeRotatedRectangle);
	
	
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
	
	
	updateSeriesForChartWithYearXAxis(elevGainPerYearChart,		elevGainPerYearXAxis,	elevGainPerYearYAxis,	{elevGainPerYearSeries},			minYear,	maxYear,	0,	elevGainPerYearMaxY,	false);
	updateSeriesForChartWithYearXAxis(numAscentsPerYearChart,	numAscentsPerYearXAxis,	numAscentsPerYearYAxis,	{numAscentsPerYearSeries},			minYear,	maxYear,	0,	numAscentsPerYearMaxY,	false);
	updateSeriesForChartWithYearXAxis(heightsScatterChart,		heightsScatterXAxis,	heightsScatterYAxis,	{peakHeightSeries, elevGainSeries},	minDate,	maxDate,	0,	heightsMaxY,			true);
}





ItemStatsEngine::ItemStatsEngine(Database* db, QVBoxLayout* statsFrameLayout) :
	StatsEngine(db),
	statsFrameLayout(statsFrameLayout),
	peakHeightHistChart(nullptr),
	peakHeightHistBarSet(nullptr),
	peakHeightHistYAxis(nullptr),
	peakHeightHistChartView(nullptr),
	elevGainHistChart(nullptr),
	elevGainHistBarSet(nullptr),
	elevGainHistYAxis(nullptr),
	elevGainHistChartView(nullptr)
{
	assert(statsFrameLayout);
}



void ItemStatsEngine::setupStatsPanel()
{
							peakHeightHistChart		= createChart("Peak height distribution", false);
	QBarCategoryAxis*		peakHeightHistXAxis		= createBarCategoryXAxis(peakHeightHistChart, {"&lt;1000", "1000s", "2000s", "3000s", "4000s", "5000s", "6000s", "7000s", "8000s"}, Qt::AlignLeft);
							peakHeightHistYAxis		= createValueYAxis(peakHeightHistChart, QString(), Qt::AlignBottom);
	QHorizontalBarSeries*	peakHeightSeries		= createHorizontalBarSeries(peakHeightHistChart, peakHeightHistXAxis, peakHeightHistYAxis);
							peakHeightHistBarSet	= createBarSet("Peak heights", peakHeightSeries);
							peakHeightHistChartView	= createChartView(peakHeightHistChart, 52 + 26 * peakHeightHistXAxis->count());
	
							elevGainHistChart		= createChart("Elevation gain distribution", false);
	QBarCategoryAxis*		elevGainHistXAxis		= createBarCategoryXAxis(elevGainHistChart, {"&lt;250", "&ge;250", "&ge;500", "&ge;750", "&ge;1000", "&ge;1250", "&ge;1500"}, Qt::AlignLeft);
							elevGainHistYAxis		= createValueYAxis(elevGainHistChart, QString(), Qt::AlignBottom);
	QHorizontalBarSeries*	elevGainSeries			= createHorizontalBarSeries(elevGainHistChart, elevGainHistXAxis, elevGainHistYAxis);
							elevGainHistBarSet		= createBarSet("Elevation gains", elevGainSeries);
							elevGainHistChartView	= createChartView(elevGainHistChart, 52 + 26 * elevGainHistXAxis->count());
	
	statsFrameLayout->addWidget(peakHeightHistChartView);
	statsFrameLayout->addWidget(elevGainHistChartView);
}

void ItemStatsEngine::updateStatsPanel(const QSet<BufferRowIndex>& selectedBufferRows)
{
	assert(peakHeightHistBarSet);
	assert(elevGainHistBarSet);
	
	
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
	QList<int> peakHeightHistogram = QList<int>(numPeakHeightCategories, 0);
	int peakHeightMaxY = 0;
	for (const int peakHeight : peakHeights) {
		int newValue = ++peakHeightHistogram[peakHeight / 1000];
		if (newValue > peakHeightMaxY) peakHeightMaxY = newValue;
	}
	
	adjustAxis(peakHeightHistYAxis, 0, peakHeightMaxY, peakHeightHistChart->plotArea().width(), 0.05);
	
	// Clear and repopulate bar set with new values
	peakHeightHistBarSet->remove(0, numPeakHeightCategories);
	for (const int numPeaks : peakHeightHistogram) {
		*peakHeightHistBarSet << numPeaks;
	}
	
	
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
	QList<int> elevGainHistogram = QList<int>(numElevGainCategories, 0);
	int elevGainMaxY = 0;
	for (const int elevGain : elevGains) {
		int elevGainClass = std::min(elevGain, 1500) / 250;
		int newValue = ++elevGainHistogram[elevGainClass];
		if (newValue > elevGainMaxY) elevGainMaxY = newValue;
	}
	
	adjustAxis(elevGainHistYAxis, 0, elevGainMaxY, elevGainHistChart->plotArea().width(), 0.05);
	
	// Clear and repopulate bar set with new values
	elevGainHistBarSet->remove(0, numElevGainCategories);
	for (const int numAscents : elevGainHistogram) {
		*elevGainHistBarSet << numAscents;
	}
}
