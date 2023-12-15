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
 * @file chart.cpp
 * 
 * This file defines the Chart class and its subclasses.
 */

#include "chart.h"

#include <QGraphicsLayout>
#include <QBarSet>



Chart::Chart(const QString& chartTitle) :
	chartTitle(chartTitle),
	chart		(nullptr),
	chartView	(nullptr)
{}

Chart::~Chart()
{
	//		chart		is deleted by chartView
	delete	chartView;
}



QChartView* Chart::getChartView() const
{
	return chartView;
}



QChart* Chart::createChart(const QString& title)
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
	chart->legend()->hide();
	return chart;
}

QValueAxis* Chart::createValueXAxis(QChart* chart, const QString& title)
{
	assert(chart);
	
	QValueAxis* xAxis = new QValueAxis();
	xAxis->setLabelFormat("%.0f");
	xAxis->setTickType(QValueAxis::TicksDynamic);
	if (!title.isEmpty()) xAxis->setTitleText(title);
	chart->addAxis(xAxis, Qt::AlignBottom);
	return xAxis;
}

QBarCategoryAxis* Chart::createBarCategoryXAxis(QChart* chart, const QStringList& categories, const Qt::AlignmentFlag alignment)
{
	QBarCategoryAxis* xAxis = new QBarCategoryAxis();
	xAxis->setCategories(categories);
	chart->addAxis(xAxis, alignment);
	return xAxis;
}

QValueAxis* Chart::createValueYAxis(QChart* chart, const QString& title, const Qt::AlignmentFlag alignment)
{
	assert(chart);
	
	QValueAxis* yAxis = new QValueAxis();
	yAxis->setLabelFormat("%.0f");
	yAxis->setTickType(QValueAxis::TicksDynamic);
	if (!title.isEmpty()) yAxis->setTitleText(title);
	chart->addAxis(yAxis, alignment);
	return yAxis;
}

SizeResponsiveChartView* Chart::createChartView(QChart* chart, int minimumHeight)
{
	assert(chart);
	
	SizeResponsiveChartView* chartView = new SizeResponsiveChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);
	if (minimumHeight >= 0) chartView->setMinimumHeight(minimumHeight);
	return chartView;
}


QHorizontalBarSeries* Chart::createHorizontalBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis)
{
	QHorizontalBarSeries* series = new QHorizontalBarSeries();
	chart->addSeries(series);
	series->attachAxis(xAxis);
	series->attachAxis(yAxis);
	return series;
}

QBarSet* Chart::createBarSet(const QString& name, QAbstractBarSeries* series)
{
	QBarSet* barSet = new QBarSet(name);
	series->append(barSet);
	return barSet;
}



QLineSeries* Chart::createLineSeries(const QString& name)
{
	QLineSeries* series = new QLineSeries();
	series->setName(name);
	return series;
}

QScatterSeries* Chart::createScatterSeries(const QString& name, int markerSize, QScatterSeries::MarkerShape marker)
{
	QScatterSeries* series = new QScatterSeries();
	series->setName(name);
	if (markerSize >= 0)	series->setMarkerSize(markerSize);
	if (marker >= 0)		series->setMarkerShape(QScatterSeries::MarkerShapeTriangle);
	series->setPen(QPen(Qt::transparent));
	return series;
}


void Chart::adjustAxis(QValueAxis* axis, qreal minValue, qreal maxValue, int chartSize, qreal rangeBufferFactor)
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
	if (interval < 1) interval = 1;
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





YearChart::YearChart(const QString& chartTitle, const QString& yAxisTitle, bool bufferXAxisRange) :
	Chart(chartTitle),
	yAxisTitle(yAxisTitle),
	bufferXAxisRange(bufferXAxisRange),
	xAxis	(nullptr),
	yAxis	(nullptr)
{
	YearChart::setup();
}

YearChart::~YearChart()
{
	// xAxis	is deleted by chart
	// yAxis	is deleted by chart
}



void YearChart::setup()
{
	chart		= createChart(chartTitle);
	xAxis		= createValueXAxis(chart);
	yAxis		= createValueYAxis(chart, yAxisTitle);
	chartView	= createChartView(chart);
	
	connect(chartView, &SizeResponsiveChartView::wasResized, this, &YearChart::updateView);
}

void YearChart::updateData(const QList<QXYSeries*>& newSeries, qreal minYear, qreal maxYear, qreal minY, qreal maxY)
{
	chart->removeAllSeries();
	for (QXYSeries* const series : newSeries) {
		chart->addSeries(series);
		series->attachAxis(xAxis);
		series->attachAxis(yAxis);
	}
	chart->legend()->setVisible(newSeries.length() > 1);
	
	this->minYear = minYear;
	this->maxYear = maxYear;
	this->minY = minY;
	this->maxY = maxY;
	updateView();
}

void YearChart::updateView()
{
	if (!chart) return;
	adjustAxis(xAxis,	minYear,	maxYear,	chart->plotArea().width(),	bufferXAxisRange ? rangeBufferFactorX : 0);
	adjustAxis(yAxis,	minY,		maxY,		chart->plotArea().height(),	rangeBufferFactorY);
}





HistogramChart::HistogramChart(const QString& chartTitle, const QStringList& xAxisCategories, const QString& barSetTitle) :
	Chart(chartTitle),
	xAxisCategories(xAxisCategories),
	barSetTitle(barSetTitle),
	xAxis		(nullptr),
	yAxis		(nullptr),
	barSeries	(nullptr),
	barSet		(nullptr)
{
	HistogramChart::setup();
}

HistogramChart::~HistogramChart()
{
	// xAxis		is deleted by chart
	// yAxis		is deleted by chart
	// barSeries	is deleted by chart
	// barSet		is deleted by barSeries
}



void HistogramChart::setup()
{
	chart		= createChart(chartTitle);
	xAxis		= createBarCategoryXAxis(chart, xAxisCategories, Qt::AlignLeft);
	yAxis		= createValueYAxis(chart, QString(), Qt::AlignBottom);
	barSeries	= createHorizontalBarSeries(chart, xAxis, yAxis);
	barSet		= createBarSet(barSetTitle, barSeries);
	chartView	= createChartView(chart, 52 + 26 * xAxis->count());
	
	connect(chartView, &SizeResponsiveChartView::wasResized, this, &HistogramChart::updateView);
}

void HistogramChart::updateData(QList<qreal> histogramData, qreal maxY)
{
	this->maxY = maxY;
	updateView();
	
	barSet->remove(0, barSet->count());
	barSet->append(histogramData);
}

void HistogramChart::updateView()
{
	if (!chart) return;
	adjustAxis(yAxis,	0,	maxY,	chart->plotArea().width(),	rangeBufferFactorY);
}
