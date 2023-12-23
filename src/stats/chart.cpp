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



/**
 * Creates a Chart.
 * 
 * @param chartTitle	The title of the chart, to be displayed above it.
 */
Chart::Chart(const QString& chartTitle) :
	chartTitle(chartTitle),
	chart		(nullptr),
	chartView	(nullptr),
	hasData(false)
{}

/**
 * Destroys the Chart.
 */
Chart::~Chart()
{
	//		chart		is deleted by chartView
	delete	chartView;
}



/**
 * Returns the chart view object for the chart, e.g. for inserting into a layout.
 * 
 * @return	The chart view object for the chart.
 */
QChartView* Chart::getChartView() const
{
	return chartView;
}



/**
 * Creates and initializes a QChart object.
 * 
 * Initialization entails setting title text, font and size, margins, layout, and hiding the legend.
 * 
 * @param title	The title of the chart.
 * @return		An initialized QChart object, of which the caller takes ownership.
 */
QChart* Chart::createChart(const QString& title)
{
	QChart* chart = new QChart();
	chart->setMargins(QMargins(0, 0, 0, 0));
	chart->layout()->setContentsMargins(0, 0, 0, 0);
	chart->legend()->layout()->setContentsMargins(0, 0, 0, 0);
	chart->setBackgroundRoundness(0);
	chart->setTitle(title);
	QFont titleFont = chart->titleFont();
	titleFont.setPointSize(titleFont.pointSize() * 1.2);
	titleFont.setBold(true);
	chart->setTitleFont(titleFont);
	chart->legend()->hide();
	return chart;
}

/**
 * Creates and initializes a QValueAxis object for a horizontal real-number x-axis.
 * 
 * Initialization entails setting title text, label formatting, tick type, and adding the axis to
 * the given chart (horizontally).
 * 
 * @param chart	The chart to add the axis to.
 * @param title	The title for the axis. Can be empty.
 * @return		An initialized QValueAxis object, of which the caller takes ownership.
 */
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

/**
 * Creates and initializes a QBarCategoryAxis object.
 * 
 * Initialization entails setting category names and adding the axis to the given chart.
 * 
 * @param chart			The chart to add the axis to.
 * @param categories	The list of category names for the axis.
 * @param alignment		The alignment for the axis.
 * @return				An initialized QBarCategoryAxis object, of which the caller takes ownership.
 */
QBarCategoryAxis* Chart::createBarCategoryXAxis(QChart* chart, const Qt::AlignmentFlag alignment, const QStringList& categories)
{
	QBarCategoryAxis* xAxis = new QBarCategoryAxis();
	if (!categories.isEmpty()) xAxis->setCategories(categories);
	chart->addAxis(xAxis, alignment);
	return xAxis;
}

/**
 * Creates and initializes a QValueAxis object for a real-number y-axis.
 * 
 * Initialization entails setting title text, label formatting, tick type, and adding the axis to
 * the given chart.
 * 
 * @param chart		The chart to add the axis to.
 * @param title		The title for the axis. Can be empty.
 * @param alignment	The alignment for the axis.
 * @return			An initialized QValueAxis object, of which the caller takes ownership.
 */
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

/**
 * Creates and initializes a SizeResponsiveChartView object.
 * 
 * Initialization entails setting render hint and minimum height.
 * 
 * @param chart			The chart to display in the chart view.
 * @param minimumHeight	The minimal height for the widget in pixels.
 * @return				An initialized SizeResponsiveChartView object, of which the caller takes ownership.
 */
SizeResponsiveChartView* Chart::createChartView(QChart* chart, int minimumHeight)
{
	assert(chart);
	
	SizeResponsiveChartView* chartView = new SizeResponsiveChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);
	if (minimumHeight >= 0) chartView->setMinimumHeight(minimumHeight);
	return chartView;
}


/**
 * Creates and initializes a QHorizontalBarSeries object.
 * 
 * Initialization entails adding the series to the given chart and attaching the given axes.
 * 
 * @param chart	The chart to display in the chart view.
 * @param xAxis	The chart's x-axis.
 * @param yAxis	The chart's y-axis.
 * @return		An initialized QHorizontalBarSeries object, of which the caller takes ownership.
 */
QHorizontalBarSeries* Chart::createHorizontalBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis)
{
	QHorizontalBarSeries* series = new QHorizontalBarSeries();
	chart->addSeries(series);
	series->attachAxis(xAxis);
	series->attachAxis(yAxis);
	return series;
}

/**
 * Creates and initializes a QBarSet object.
 * 
 * Initialization entails setting the name and adding the bar set to the given bar series.
 * 
 * @param name		The label text for the bar set.
 * @param series	The bar series to add the bar set to.
 * @return			An initialized QBarSet object, of which the caller takes ownership.
 */
QBarSet* Chart::createBarSet(const QString& name, QAbstractBarSeries* series)
{
	QBarSet* barSet = new QBarSet(name);
	series->append(barSet);
	return barSet;
}



/**
 * Creates and initializes a QLineSeries object.
 * 
 * Initialization entails setting the name.
 * 
 * @param name	The label text for the line series.
 * @return		An initialized QLineSeries object, of which the caller takes ownership.
 */
QLineSeries* Chart::createLineSeries(const QString& name)
{
	QLineSeries* series = new QLineSeries();
	if (!name.isEmpty()) series->setName(name);
	return series;
}

/**
 * Creates and initializes a QScatterSeries object.
 * 
 * Initialization entails setting the name, and marker size, shape, and outline.
 * 
 * @param name			The label text for the scatter series.
 * @param markerSize	The marker size for the scatter series.
 * @param markerShape	The marker shape for the scatter series.
 * @return				An initialized QScatterSeries object, of which the caller takes ownership.
 */
QScatterSeries* Chart::createScatterSeries(const QString& name, int markerSize, QScatterSeries::MarkerShape markerShape)
{
	QScatterSeries* series = new QScatterSeries();
	series->setName(name);
	if (markerSize >= 0)	series->setMarkerSize(markerSize);
	if (markerShape >= 0)	series->setMarkerShape(markerShape);
	series->setPen(QPen(Qt::transparent));
	return series;
}


/**
 * Adjusts the range and tick placement for the given axis, based on the given range information.
 * 
 * Responsive to size of the chart view widget.
 * 
 * @param axis				The axis to adjust range and tick placement for.
 * @param minValue			The minimum value on the axis included in the current dataset.
 * @param maxValue			The maximum value on the axis included in the current dataset.
 * @param chartSize			The size of the chart's plot area in the direction of the given axis (h/v) in pixels.
 * @param rangeBufferFactor	The fraction of the range to add to the edge on either side as a buffer. No buffer is added when that edge is equal to zero.
 */
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

/**
 * Resets range and tick placement for the given axis, so that it only shows the 0 tick.
 */
void Chart::resetAxis(QValueAxis* axis)
{
	axis->setRange(0, 1);
	axis->setTickAnchor(0);
	axis->setTickInterval(2);
	axis->setMinorTickCount(0);
}





/**
 * Creates a YearChart.
 * 
 * @param chartTitle		The title of the chart, to be displayed above it.
 * @param yAxisTitle		The label text for the y-axis.
 * @param bufferXAxisRange	The fraction of the x-axis range to add as buffer.
 */
YearChart::YearChart(const QString& chartTitle, const QString& yAxisTitle, bool bufferXAxisRange) :
	Chart(chartTitle),
	yAxisTitle(yAxisTitle),
	bufferXAxisRange(bufferXAxisRange),
	xAxis	(nullptr),
	yAxis	(nullptr),
	minYear(0),
	maxYear(0),
	minY(0),
	maxY(0)
{
	YearChart::setup();
	YearChart::reset();
}

/**
 * Destroys the YearChart.
 */
YearChart::~YearChart()
{
	// xAxis	is deleted by chart
	// yAxis	is deleted by chart
}



/**
 * Performs the setup for the YearChart during/after construction.
 * 
 * Not to be called more than once (will cause memory leaks).
 */
void YearChart::setup()
{
	chart		= createChart(chartTitle);
	xAxis		= createValueXAxis(chart);
	yAxis		= createValueYAxis(chart, yAxisTitle);
	chartView	= createChartView(chart);
	
	// Interactivity
	chartView->setInteractive(true);
	chartView->setRubberBand(QChartView::RectangleRubberBand);
	
	connect(chartView, &SizeResponsiveChartView::wasResized,			this,	&YearChart::updateView);
	connect(chartView, &SizeResponsiveChartView::receivedDoubleClick,	this,	&YearChart::resetZoom);
}

/**
 * Removes all data from the chart.
 */
void YearChart::reset()
{
	chart->removeAllSeries();
	hasData = false;
	this->minYear = 0;
	this->maxYear = 0;
	this->minY = 0;
	this->maxY = 0;
	resetAxis(xAxis);
	resetAxis(yAxis);
}

/**
 * Replaces the displayed data and stores range information for future view updates.
 * 
 * Performs a view update before replacing the data and sets the legend to visible iff more than one
 * series was added.
 * 
 * @param newSeries	A list of data series to display in the chart.
 * @param minYear	The minimum x value among all given data series.
 * @param maxYear	The maximum x value among all given data series.
 * @param minY		The minimum y value among all given data series.
 * @param maxY		The maximum y value among all given data series.
 */
void YearChart::updateData(const QList<QXYSeries*>& newSeries, qreal minYear, qreal maxYear, qreal minY, qreal maxY)
{
	bool noData = true;
	for (QXYSeries* const series : newSeries) {
		if (!series->points().isEmpty()) {
			noData = false;
			break;
		}
	}
	if (noData) {
		reset();
		return;
	}
	assert(minYear <= maxYear);
	assert(minY <= maxY);
	
	if (maxYear - minYear < 1) {
		qDebug() << minYear << maxYear;
		qreal buffer = 0.5 * (1 - (maxYear - minYear));
		minYear -= buffer;
		maxYear += buffer;
		qDebug() << buffer << minYear << maxYear;
	}
	
	this->minYear = minYear;
	this->maxYear = maxYear;
	this->minY = minY;
	this->maxY = maxY;
	hasData = true;
	updateView();
	chart->legend()->setVisible(newSeries.length() > 1);
	
	chart->removeAllSeries();
	for (QXYSeries* const series : newSeries) {
		chart->addSeries(series);
		series->attachAxis(xAxis);
		series->attachAxis(yAxis);
	}
}

/**
 * Updates the chart layout, e.g. tick spacing, without changing the displayed data.
 */
void YearChart::updateView()
{
	if (!hasData) return;
	adjustAxis(xAxis,	minYear,	maxYear,	chart->plotArea().width(),	bufferXAxisRange ? rangeBufferFactorX : 0);
	adjustAxis(yAxis,	minY,		maxY,		chart->plotArea().height(),	rangeBufferFactorY);
}

/**
 * Resets zoom level set by user.
 * 
 * To be called when user requests to reset the chart view.
 */
void YearChart::resetZoom()
{
	if (!hasData) return;
	chart->zoomReset();
}





/**
 * Creates a HistogramChart.
 * 
 * @param chartTitle		The title of the chart, to be displayed above it.
 * @param xAxisCategories	The list of translated category names for the x-axis.
 */
HistogramChart::HistogramChart(const QString& chartTitle, const QStringList& xAxisCategories) :
	Chart(chartTitle),
	xAxisCategories(xAxisCategories),
	xAxis		(nullptr),
	yAxis		(nullptr),
	barSeries	(nullptr),
	barSet		(nullptr)
{
	HistogramChart::setup();
	HistogramChart::reset();
}

/**
 * Destroys the HistogramChart.
 */
HistogramChart::~HistogramChart()
{
	// xAxis		is deleted by chart
	// yAxis		is deleted by chart
	// barSeries	is deleted by chart
	// barSet		is deleted by barSeries
}



/**
 * Performs the setup for the HistogramChart during/after construction.
 * 
 * Not to be called more than once (will cause memory leaks).
 */
void HistogramChart::setup()
{
	chart		= createChart(chartTitle);
	xAxis		= createBarCategoryXAxis(chart, Qt::AlignLeft, xAxisCategories);
	yAxis		= createValueYAxis(chart, QString(), Qt::AlignBottom);
	barSeries	= createHorizontalBarSeries(chart, xAxis, yAxis);
	barSet		= createBarSet(QString(), barSeries);
	chartView	= createChartView(chart, 52 + 26 * xAxis->count());
	
	connect(chartView, &SizeResponsiveChartView::wasResized, this, &HistogramChart::updateView);
}

/**
 * Removes all data from the chart.
 */
void HistogramChart::reset()
{
	barSet->remove(0, barSet->count());
	hasData = false;
	this->maxY = 0;
	resetAxis(yAxis);
}

/**
 * Replaces the displayed data and stores range information for future view updates.
 * 
 * Performs a view update before replacing the data.
 * 
 * @param histogramData	A list of data points to display in the chart. The length of the list must match the number of categories.
 * @param maxY			The maximum y value in the given data.
 */
void HistogramChart::updateData(QList<qreal> histogramData, qreal maxY)
{
	bool noData = true;
	if (!histogramData.isEmpty()) {
		for (const qreal point : histogramData) {
			if (point > 0) {
				noData = false;
				break;
			}
		}
	}
	if (noData) {
		reset();
		return;
	}
	assert(maxY > 0);
	
	this->maxY = maxY;
	hasData = true;
	updateView();
	
	barSet->remove(0, barSet->count());
	barSet->append(histogramData);
}

/**
 * Updates the chart layout, e.g. tick spacing, without changing the displayed data.
 */
void HistogramChart::updateView()
{
	if (!hasData) return;
	adjustAxis(yAxis,	0,	maxY,	chart->plotArea().width(),	rangeBufferFactorY);
}





/**
 * Creates a TopNChart.
 * 
 * @param n				The number of items to compare in the chart.
 * @param chartTitle	The title of the chart, to be displayed above it.
 * @param yAxisTitle	The label text for the y-axis.
 */
TopNChart::TopNChart(int n, const QString& chartTitle, const QString& yAxisTitle) :
	Chart(chartTitle),
	n(n),
	yAxisTitle(yAxisTitle),
	xAxis		(nullptr),
	yAxis		(nullptr),
	barSeries	(nullptr),
	barSet		(nullptr)
{
	TopNChart::setup();
	TopNChart::reset();
}

/**
 * Destroys the TopNChart.
 */
TopNChart::~TopNChart()
{
	// xAxis		is deleted by chart
	// yAxis		is deleted by chart
	// barSeries	is deleted by chart
	// barSet		is deleted by barSeries
}



/**
 * Performs the setup for the TopNChart during/after construction.
 * 
 * Not to be called more than once (will cause memory leaks).
 */
void TopNChart::setup()
{
	chart		= createChart(chartTitle);
	xAxis		= createBarCategoryXAxis(chart, Qt::AlignLeft);
	yAxis		= createValueYAxis(chart, yAxisTitle, Qt::AlignBottom);
	barSeries	= createHorizontalBarSeries(chart, xAxis, yAxis);
	barSet		= createBarSet(QString(), barSeries);
	chartView	= createChartView(chart, 52 + 26 * n);
	
	connect(chartView, &SizeResponsiveChartView::wasResized, this, &TopNChart::updateView);
}

/**
 * Removes all data from the chart.
 */
void TopNChart::reset()
{
	barSet->remove(0, barSet->count());
	xAxis->setCategories({});
	hasData = false;
	this->maxY = 0;
	resetAxis(yAxis);
}

/**
 * Replaces the displayed data and stores range information for future view updates.
 * 
 * Performs a view update before replacing the data.
 * 
 * @param histogramData	A list of data points to display in the chart. The length of the list must match the number of categories.
 */
void TopNChart::updateData(QStringList labels, QList<qreal> values)
{
	assert(labels.size() == values.size());
	assert(labels.size() <= n);
	
	if (values.isEmpty()) {
		reset();
		return;
	}
	
	maxY = values.first();
	assert(maxY >= 0);
	
	// Handle duplicate labels (otherwise the duplicates will be missing)
	renameDuplicates(labels);
	
	// Reverse lists (because entries will be displayed bottom-to-top)
	std::reverse(labels.begin(), labels.end());
	std::reverse(values.begin(), values.end());
	
	barSet->remove(0, barSet->count());
	xAxis->setCategories(labels);
	barSet->append(values);
	
	hasData = true;
	updateView();
}

/**
 * Updates the chart layout, e.g. tick spacing, without changing the displayed data.
 */
void TopNChart::updateView()
{
	if (!hasData) return;
	adjustAxis(yAxis,	0,	maxY,	chart->plotArea().width(),	rangeBufferFactorY);
}



/**
 * Ensures that the given string list has no duplicates in it by appending " (1)" and so on.
 * 
 * @param list	List of strings to be manipulated in place.
 */
void TopNChart::renameDuplicates(QStringList& list)
{
	for (int i = 0; i < list.size(); i++) {
		const QString originalLabel = list[i];
		
		int counter = 1;
		for (int j = i + 1; j < list.size(); j++) {
			if (list.at(j) == originalLabel) {
				if (counter == 1) list[i] += " (1)";
				list[j] += " (" + QString::number(++counter) + ")";
			}
		}
	}
}
