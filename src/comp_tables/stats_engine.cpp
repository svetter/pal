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
 * This file defines the GeneralStatsEngine class.
 */

#include "stats_engine.h"

#include <QGraphicsLayout>



GeneralStatsEngine::GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr) :
	db(db),
	statisticsTabLayoutPtr(statisticsTabLayoutPtr)
{
	assert(statisticsTabLayoutPtr);
}



void GeneralStatsEngine::setupStatsTab()
{
	elevGainPerYearChart	= createChart("Elevation gain sum per year", false);
	elevGainPerYearXAxis	= createXAxis(elevGainPerYearChart);
	elevGainPerYearYAxis	= createYAxis(elevGainPerYearChart, "km");
	QChartView* elevGainPerYearChartView = createChartView(elevGainPerYearChart);
	
	numAscentsPerYearChart	= createChart("Number of scaled peaks per year", false);
	numAscentsPerYearXAxis	= createXAxis(numAscentsPerYearChart);
	numAscentsPerYearYAxis	= createYAxis(numAscentsPerYearChart, "Number of peaks");
	QChartView* numAscentsPerYearChartView = createChartView(numAscentsPerYearChart);
	
	heightsScatterChart		= createChart("All elevation gains and peak heights over time", true);
	heightsScatterXAxis		= createXAxis(heightsScatterChart);
	heightsScatterYAxis		= createYAxis(heightsScatterChart, "m");
	QChartView* heightsScatterChartView = createChartView(heightsScatterChart);
	
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
				yearElevGainSums[year] += elevGain;
			}
			if (ascent->peakID.isValid()) {
				const Peak* const peak = db->getPeak(FORCE_VALID(ascent->peakID));
				if (peak->heightSpecified()) {
					int peakHeight = peak->height;
					peakHeightSeries->append(dateReal, peakHeight);
				}
			}
		}
		
		delete ascent;
	}
	
	for (int year = minYear; year <= maxYear; year++) {
		int elevGainSum	= yearElevGainSums	.contains(year) ? yearElevGainSums	[year] : 0;
		int numAscents	= yearNumAscents	.contains(year) ? yearNumAscents	[year] : 0;
		elevGainPerYearSeries	->append(year, (qreal) elevGainSum / 1000);
		numAscentsPerYearSeries	->append(year, numAscents);
	}
	
	
	updateSeriesForChartWithYearXAxis(elevGainPerYearChart,		elevGainPerYearXAxis,	elevGainPerYearYAxis,	{elevGainPerYearSeries},			minYear,	maxYear,	5);
	updateSeriesForChartWithYearXAxis(numAscentsPerYearChart,	numAscentsPerYearXAxis,	numAscentsPerYearYAxis,	{numAscentsPerYearSeries},			minYear,	maxYear,	5);
	updateSeriesForChartWithYearXAxis(heightsScatterChart,		heightsScatterXAxis,	heightsScatterYAxis,	{peakHeightSeries, elevGainSeries},	minDate,	maxDate,	2);
}



QChart* GeneralStatsEngine::createChart(const QString& title, bool displayLegend)
{
	QChart* chart = new QChart();
	chart->setMargins(QMargins(0, 0, 0, 0));
	chart->layout()->setContentsMargins(0, 0, 0, 0);
	chart->setBackgroundRoundness(0);
	chart->setTitle(title);
	QFont titleFont = QFont();
	titleFont.setPointSize(titleFont.pointSize() * 1.2);
	titleFont.setBold(true);
	chart->setTitleFont(titleFont);
	if (!displayLegend) chart->legend()->hide();
	return chart;
}

QValueAxis* GeneralStatsEngine::createXAxis(QChart* chart, const QString& title)
{
	assert(chart);
	
	QValueAxis* xAxis = new QValueAxis();
	xAxis->setLabelFormat("%.0f");
	xAxis->setTickType(QValueAxis::TicksDynamic);
	if (!title.isEmpty()) xAxis->setTitleText("km");
	chart->addAxis(xAxis, Qt::AlignBottom);
	return xAxis;
}

QValueAxis* GeneralStatsEngine::createYAxis(QChart* chart, const QString& title)
{
	assert(chart);
	
	QValueAxis* yAxis = new QValueAxis();
	yAxis->setLabelFormat("%.0f");
	if (!title.isEmpty()) yAxis->setTitleText(title);
	chart->addAxis(yAxis, Qt::AlignLeft);
	return yAxis;
}

QChartView* GeneralStatsEngine::createChartView(QChart* chart)
{
	assert(chart);
	
	QChartView* chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);
	return chartView;
}


void GeneralStatsEngine::addChartsToLayout(QBoxLayout* layout, const QList<QChartView*>& charts, QList<int> stretchFactors)
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



QLineSeries* GeneralStatsEngine::createLineSeries(const QString& name)
{
	QLineSeries* series = new QLineSeries();
	series->setName(name);
	return series;
}

QScatterSeries* GeneralStatsEngine::createScatterSeries(const QString& name, int markerSize, QScatterSeries::MarkerShape marker)
{
	QScatterSeries* series = new QScatterSeries();
	series->setName(name);
	if (markerSize >= 0)	series->setMarkerSize(markerSize);
	if (marker >= 0)		series->setMarkerShape(QScatterSeries::MarkerShapeTriangle);
	series->setPen(QPen(Qt::transparent));
	return series;
}


void GeneralStatsEngine::updateSeriesForChartWithYearXAxis(QChart* chart, QValueAxis* xAxis, QValueAxis* yAxis, const QList<QXYSeries*>& newSeries, qreal minYear, qreal maxYear, int yearInterval)
{
	chart->removeAllSeries();
	for (QXYSeries* const series : newSeries) {
		chart->addSeries(series);
		series->attachAxis(xAxis);
		series->attachAxis(yAxis);
	}
	
	xAxis->setRange(minYear, maxYear);
	xAxis->setTickInterval(yearInterval);
	xAxis->setTickAnchor((int) minYear / yearInterval * yearInterval);
	xAxis->setMinorTickCount(yearInterval - 1);
	
	yAxis->applyNiceNumbers();
}
