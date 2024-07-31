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
 * @file chart.cpp
 * 
 * This file defines the Chart class and its subclasses.
 */

#include "chart.h"

#include <QGraphicsLayout>
#include <QBarSet>
#include <QDateTime>



/**
 * Creates a Chart.
 * 
 * @param chartTitle	The title of the chart, to be displayed above it.
 */
Chart::Chart(const QString& chartTitle) :
	chartTitle(chartTitle),
	chart		(nullptr),
	chartView	(nullptr),
	hasData			(false),
	usePinnedRanges	(false)
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
 * Sets the flag for whether the pinned ranges should currently be used for the chart.
 * 
 * If data is already set, the view is updated after a change.
 * 
 * @param pinned	Whether to use the pinned ranges.
 */
void Chart::setUsePinnedRanges(bool pinned)
{
	if (pinned == usePinnedRanges) return;
	
	usePinnedRanges = pinned;
	
	if (hasData) updateView();
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
 * Creates and initializes a QDateTimeAxis object for a horizontal date x-axis.
 * 
 * Initialization entails setting title text, label formatting, and adding the axis to the given
 * chart (horizontally).
 * 
 * @param chart	The chart to add the axis to.
 * @param title	The title for the axis. Can be empty.
 * @return		An initialized QDateTimeAxis object, of which the caller takes ownership.
 */
QDateTimeAxis* Chart::createDateXAxis(QChart* chart, const QString& title)
{
	assert(chart);
	
	QDateTimeAxis* xAxis = new QDateTimeAxis();
	xAxis->setFormat("dd.MM.yyyy");
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
 * Creates and initializes a QBarSeries object.
 * 
 * Initialization entails setting label visibility, adding the series to the given chart and
 * attaching the given axes.
 * 
 * @param chart				The chart to display in the chart view.
 * @param xAxis				The chart's x-axis.
 * @param yAxis				The chart's y-axis.
 * @param showValueLabels	Whether to show labels stating the value of each bar.
 * @return					An initialized QHorizontalBarSeries object, of which the caller takes ownership.
 */
QBarSeries* Chart::createBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis, bool showValueLabels)
{
	QBarSeries* series = new QBarSeries();
	series->setLabelsVisible(showValueLabels);
	series->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
	series->setLabelsAngle(-90);
	series->setLabelsPrecision(4);
	chart->addSeries(series);
	series->attachAxis(xAxis);
	series->attachAxis(yAxis);
	return series;
}

/**
 * Creates and initializes a QHorizontalBarSeries object.
 * 
 * Initialization entails setting label visibility, adding the series to the given chart and
 * attaching the given axes.
 * 
 * @param chart				The chart to display in the chart view.
 * @param xAxis				The chart's x-axis.
 * @param yAxis				The chart's y-axis.
 * @param showValueLabels	Whether to show labels stating the value of each bar.
 * @return					An initialized QHorizontalBarSeries object, of which the caller takes ownership.
 */
QHorizontalBarSeries* Chart::createHorizontalBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis, bool showValueLabels)
{
	QHorizontalBarSeries* series = new QHorizontalBarSeries();
	series->setLabelsVisible(showValueLabels);
	series->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
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
	series->setUseOpenGL(false);	// Enabling OpenGL causes severe slowdowns when adding series to chart
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
	series->setUseOpenGL(false);	// Enabling OpenGL causes severe slowdowns when adding series to chart
	return series;
}


/**
 * Adjusts the range and tick placement for the given real-value axis, based on the given range
 * information.
 * 
 * Responsive to size of the chart view widget.
 * 
 * @param axis				The axis to adjust range and tick placement for.
 * @param minValue			The minimum value on the axis included in the current dataset.
 * @param maxValue			The maximum value on the axis included in the current dataset.
 * @param chartSize			The size of the chart's plot area in the direction of the given axis (h/v) in pixels.
 * @param rangeBufferFactor	The fraction of the range to add to the edge on either side as a buffer. No buffer is added when that edge is equal to zero.
 * @param isTimeAxis		Whether the axis is a time axis. If true, the axis will show minor ticks for every month at small ranges.
 */
void Chart::adjustAxis(QValueAxis* axis, qreal minValue, qreal maxValue, int chartSize, qreal rangeBufferFactor, bool isTimeAxis)
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
	if (interval < 2) {
		if (isTimeAxis) {
			minorCount = log10times3 < -1 ? 11 : 3;
		} else {
			minorCount = 0;
		}
	}
	int anchor = (int) minValue / interval * interval;
	
	// Add buffer on the beginning and end of the range
	qreal rangeBuffer = range * rangeBufferFactor;
	qreal rangeMin = minValue == 0 ? 0 : (minValue - rangeBuffer);
	qreal rangeMax = maxValue == 0 ? 0 : (maxValue + rangeBuffer);
	
	// Determine order in which to best set new parameters
	const bool setRangeFirst = (axis->max() - axis->min()) / interval > (rangeMax - rangeMin) / axis->tickInterval();
	
	axis->setMinorTickCount(0);
	if (setRangeFirst) {
		axis->setRange(rangeMin, rangeMax);
	}
	axis->setTickAnchor(anchor);
	axis->setTickInterval(interval);
	if (!setRangeFirst) {
		// New tick count MUST BE set BEFORE range to avoid huge performance tank when increasing range
		axis->setRange(rangeMin, rangeMax);
	}
	axis->setMinorTickCount(minorCount);
}

/**
 * Adjusts the range and tick count for the given date axis, based on the given range information.
 * 
 * Responsive to size of the chart view widget.
 * 
 * @param axis		The axis to adjust range and tick placement for.
 * @param minValue	The minimum value on the axis included in the current dataset.
 * @param maxValue	The maximum value on the axis included in the current dataset.
 * @param chartSize	The size of the chart's plot area in the direction of the given axis (h/v) in pixels.
 */
void Chart::adjustAxis(QDateTimeAxis* axis, QDate minValue, QDate maxValue, int chartSize)
{
	assert(axis);
	assert(minValue <= maxValue);
	if (chartSize <= 0) return;
	
	// Find appropriate number of ticks (not counting the rightmost tick for now)
	const int numDays = minValue.daysTo(maxValue) + 1;
	const int tickCountFromSize = chartSize / pixelsPerTick;
	int tickCount = std::min(numDays, tickCountFromSize);
	
	// Make sure that all ticks are at midnight
	int bufferDays = 0;
	if (numDays > tickCount) {
		// Days per tick should be an integer
		bufferDays = tickCount - numDays % tickCount;
	}
	assert((numDays + bufferDays) % tickCount == 0);
	const int daysPerTick = (numDays + bufferDays) / tickCount;
	
	// Reduce buffer and number of ticks if there are ticks which only mark buffer days
	const int onlyBufferTicks = bufferDays / daysPerTick;
	bufferDays -= onlyBufferTicks * daysPerTick;
	tickCount -= onlyBufferTicks;
	
	const int bufferDaysLeft	= bufferDays / 2;
	const int bufferDaysRight	= (bufferDays + 1) / 2;	// Put odd buffer day on the right
	
	QDateTime rangeMin = QDateTime(minValue.addDays(-bufferDaysLeft),		QTime(0, 0));
	QDateTime rangeMax = QDateTime(maxValue.addDays( bufferDaysRight + 1),	QTime(0, 0));
	
	axis->setRange(rangeMin, rangeMax);
	axis->setTickCount(tickCount + 1);
	axis->setVisible(true);
}

/**
 * Resets range and tick placement for the given axis, so that it only shows the 0 tick or no labels
 * at all.
 * 
 * @param axis		The axis to reset.
 * @param show0Tick	Whether to show the 0 tick.
 */
void Chart::resetAxis(QValueAxis* axis, bool show0Tick)
{
	axis->setRange(0, 1);
	axis->setTickAnchor(show0Tick ? 0 : -1);
	axis->setTickInterval(3);
	axis->setMinorTickCount(0);
}

/**
 * Resets range and tick placement for the given axis and hides it.
 * 
 * @param axis	The axis to reset.
 */
void Chart::resetAxis(QDateTimeAxis* axis)
{
	QDateTime today = QDateTime(QDateTime::currentDateTime().date(), QTime());
	axis->setRange(today.addDays(-1), today.addDays(1));
	axis->setTickCount(3);
	axis->setVisible(false);
}





/**
 * Creates a YearBarChart.
 * 
 * @param chartTitle	The title of the chart, to be displayed above it.
 * @param yAxisTitle	The label text for the y-axis.
 */
YearBarChart::YearBarChart(const QString& chartTitle, const QString& yAxisTitle) :
	Chart(chartTitle),
	yAxisTitle(yAxisTitle),
	xAxis		(nullptr),
	yAxis		(nullptr),
	barSeries	(nullptr),
	barSet		(nullptr),
	minYear	{0, 0},
	maxYear	{0, 0},
	maxY	{0, 0}
{
	YearBarChart::setup();
	YearBarChart::reset();
}

/**
 * Destroys the YearBarChart.
 */
YearBarChart::~YearBarChart()
{
	// xAxis		is deleted by chart
	// yAxis		is deleted by chart
	// barSeries	is deleted by chart
	// barSet		is deleted by barSeries
}



/**
 * Performs the setup for the YearBarChart during/after construction.
 * 
 * Not to be called more than once (will cause memory leaks).
 */
void YearBarChart::setup()
{
	chart		= createChart(chartTitle);
	//xAxis		= createBarCategoryXAxis(chart, Qt::AlignBottom);
	xAxis		= createValueXAxis(chart);
	yAxis		= createValueYAxis(chart, yAxisTitle, Qt::AlignLeft);
	barSeries	= createBarSeries(chart, xAxis, yAxis);
	barSet		= createBarSet(QString(), barSeries);
	chartView	= createChartView(chart);
	
	connect(chartView, &SizeResponsiveChartView::wasResized, this, &YearBarChart::updateView);
}

/**
 * Clears displayed data from the chart.
 */
void YearBarChart::clear()
{
	barSet->remove(0, barSet->count());
}

/**
 * Removes all data, resets pinned ranges and shows an empty chart.
 */
void YearBarChart::reset()
{
	barSet->remove(0, barSet->count());
	hasData = false;
	for (const bool p : {false, true}) {
		minYear	[p] = 0;
		maxYear	[p] = 0;
		maxY	[p] = 0;
	}
	resetAxis(yAxis, true);
}

/**
 * Replaces the displayed data and stores range information for future view updates.
 * 
 * Performs a view update after replacing the data.
 * 
 * @param newData			The data to display in the chart. The length of the list must match the number of years between newMinYear and newMaxYear.
 * @param newMinYear		The minimum year in the data.
 * @param newMaxYear		The maximum year in the data.
 * @param newMaxY			The maximum y value in the given data.
 * @param setPinnedRanges	Whether to store the given range information as pinned range data.
 */
void YearBarChart::updateData(const QList<qreal>& newData, int newMinYear, int newMaxYear, qreal newMaxY, bool setPinnedRanges)
{
	if (newData.isEmpty()) {
		clear();
		return;
	}
	assert(newData.size() == newMaxYear - newMinYear + 1);
	assert(newMaxY >= 0);
	
	barSet->remove(0, barSet->count());
	barSet->append(QList<qreal>(newMinYear, 0));
	barSet->append(newData);
	
	for (int p = 0; p < (setPinnedRanges ? 2 : 1); p++) {
		minYear	[p] = newMinYear;
		maxYear	[p] = newMaxYear;
		maxY	[p] = newMaxY;
	}
	hasData = true;
	updateView();
}

/**
 * Updates the chart layout, e.g. tick spacing, without changing the displayed data.
 */
void YearBarChart::updateView()
{
	if (!hasData) return;
	
	const bool p = usePinnedRanges;
	adjustAxis(xAxis,	minYear[p] - 0.5,	maxYear[p] + 0.5,	chart->plotArea().width(),	rangeBufferFactorX);
	adjustAxis(yAxis,	0,					maxY[p],			chart->plotArea().height(),	rangeBufferFactorY);
}





/**
 * Creates a new DateScatterSeries.
 *
 * @param name			The translated label for the series.
 * @param markerSize	The size of the markers for the series.
 * @param markerShape	The shape of the markers for the series.
 */
DateScatterSeries::DateScatterSeries(const QString& name, int markerSize, QScatterSeries::MarkerShape markerShape) :
	name(name),
	markerSize(markerSize),
	markerShape(markerShape),
	data(QList<QPair<QDateTime, qreal>>())
{}



/**
 * Returns a new QScatterSeries object with the properties stored in this DateScatterSeries.
 * 
 * @return	A new QScatterSeries object with the properties of this DateScatterSeries.
 */
QScatterSeries* DateScatterSeries::createScatterSeries() const
{
	return Chart::createScatterSeries(name, markerSize, markerShape);
}





/**
 * Creates a TimeScatterChart.
 * 
 * @param chartTitle	The title of the chart, to be displayed above it.
 * @param yAxisTitle	The label text for the y-axis.
 */
TimeScatterChart::TimeScatterChart(const QString& chartTitle, const QString& yAxisTitle) :
	Chart(chartTitle),
	yAxisTitle(yAxisTitle),
	xAxisDate	(nullptr),
	xAxisValue	(nullptr),
	yAxis		(nullptr),
	xySeries	{QList<QXYSeries*>(), QList<QXYSeries*>()},
	lowRange	{false, false},
	minDate		{QDate(), QDate()},
	maxDate		{QDate(), QDate()},
	minRealYear	{0, 0},
	maxRealYear	{0, 0},
	maxY		{0, 0}
{
	TimeScatterChart::setup();
	TimeScatterChart::reset();
}

/**
 * Destroys the TimeScatterChart.
 */
TimeScatterChart::~TimeScatterChart()
{
	// xAxis	is deleted by chart
	// yAxis	is deleted by chart
}



/**
 * Performs the setup for the TimeScatterChart during/after construction.
 * 
 * Not to be called more than once (will cause memory leaks).
 */
void TimeScatterChart::setup()
{
	chart		= createChart(chartTitle);
	xAxisDate	= createDateXAxis(chart);
	xAxisValue	= createValueXAxis(chart);
	yAxis		= createValueYAxis(chart, yAxisTitle);
	chartView	= createChartView(chart);
	
	// Interactivity
	chartView->setInteractive(true);
	chartView->setRubberBand(QChartView::RectangleRubberBand);
	
	connect(chartView, &SizeResponsiveChartView::wasResized,			this,	&TimeScatterChart::updateView);
	connect(chartView, &SizeResponsiveChartView::receivedDoubleClick,	this,	&TimeScatterChart::resetZoom);
}

/**
 * Clears displayed data from the chart.
 */
void TimeScatterChart::clear()
{
	for (const bool p : {false, true}) {
		xySeries[p].clear();
	}
	if (!usePinnedRanges) {
		resetAxis(xAxisDate);
		resetAxis(xAxisValue, false);
		xAxisDate->setVisible(false);
		xAxisValue->setVisible(true);
		resetAxis(yAxis, true);
	}
}

/**
 * Removes all data, resets pinned ranges and shows an empty chart.
 */
void TimeScatterChart::reset()
{
	chart->removeAllSeries();
	hasData = false;
	for (const bool p : {false, true}) {
		xySeries	[p].clear();
		minDate		[p] = QDate();
		maxDate		[p] = QDate();
		minRealYear	[p] = 0;
		maxRealYear	[p] = 0;
		maxY		[p] = 0;
	}
	resetAxis(xAxisDate);
	resetAxis(xAxisValue, false);
	xAxisDate->setVisible(false);
	xAxisValue->setVisible(true);
	resetAxis(yAxis, true);
}

/**
 * Replaces the displayed data and stores range information for future view updates.
 * 
 * Performs a view update after generating and storing the new data series.
 * 
 * @param seriesData		A list of data series to display in the chart.
 * @param newMinDate		The minimum x-value (date) among all given data series.
 * @param newMaxDate		The maximum x-value (date) among all given data series.
 * @param newMaxY			The maximum y value among all given data series.
 * @param setPinnedRanges	Whether to store the given range information as pinned range data.
 */
void TimeScatterChart::updateData(const QList<DateScatterSeries*>& seriesData, QDate newMinDate, QDate newMaxDate, qreal newMaxY, bool setPinnedRanges)
{
	bool noData = true;
	for (const DateScatterSeries* const series : seriesData) {
		if (!series->data.isEmpty()) {
			noData = false;
			break;
		}
	}
	if (noData && !usePinnedRanges) {
		clear();
		return;
	}
	assert(newMinDate <= newMaxDate);
	assert(newMaxY >= 0);
	
	auto getYearReal = [](QDateTime dateTime) {
		const QDateTime startOfYear		= QDateTime(QDate(dateTime.date().year(),		1, 1), QTime(0, 0));
		const QDateTime startOfNextYear	= QDateTime(QDate(dateTime.date().year() + 1,	1, 1), QTime(0, 0));
		const qint64 dateTimeSecs			= dateTime.toSecsSinceEpoch();
		const qint64 startOfYearSecs		= startOfYear.toSecsSinceEpoch();
		const qint64 startOfNextYearSecs	= startOfNextYear.toSecsSinceEpoch();
		return (qreal) (dateTimeSecs - startOfYearSecs) / (startOfNextYearSecs - startOfYearSecs) + dateTime.date().year();
	};
	
	for (int p = 0; p < (setPinnedRanges ? 2 : 1); p++) {
		lowRange[p] = newMaxDate.year() - newMinDate.year() < 2;
		
		minDate[p] = newMinDate;
		maxDate[p] = newMaxDate;
		minRealYear[p] = getYearReal(QDateTime(newMinDate,				QTime(0, 0)));
		maxRealYear[p] = getYearReal(QDateTime(newMaxDate.addDays(1),	QTime(0, 0)));
		if (lowRange[p]) {
			const qreal buffer = 0.5 * (2 - (maxRealYear[p] - minRealYear[p]));
			if (buffer < 0)
			assert(buffer >= 0);
			minRealYear[p] -= buffer;
			maxRealYear[p] += buffer;
		}
		assert(minRealYear[p] <= maxRealYear[p]);
		maxY[p] = newMaxY;
	}
	
	// Convert dates to real representation and create series
	for (int p = 0; p < 2; p++) {
		qDeleteAll(xySeries[p]);
		xySeries[p].clear();
	}
	for (const DateScatterSeries* const series : seriesData) {
		QScatterSeries* newQSeries[2] = {
			createScatterSeries(series->name, series->markerSize, series->markerShape),
			createScatterSeries(series->name, series->markerSize, series->markerShape)
		};
		
		for (const auto& [dateTime, yValue] : series->data) {
			for (int p = 0; p < 2; p++) {
				qreal xValue;
				if (lowRange[p]) {
					xValue = dateTime.toMSecsSinceEpoch();
				} else {
					xValue = getYearReal(dateTime);
				}
				
				newQSeries[p]->append(xValue, yValue);
			}
		}
		
		xySeries[0].append(newQSeries[0]);
		xySeries[1].append(newQSeries[1]);
	}
	
	hasData = true;
	updateView();
}

/**
 * Updates the chart layout, e.g. tick spacing, without changing the displayed data.
 * 
 * Removes all series from the chart and adds them again, according to the current range settings.
 */
void TimeScatterChart::updateView()
{
	if (!hasData) return;
	
	chartView->setUpdatesEnabled(false);
	
	const bool p = usePinnedRanges;
	adjustAxis(xAxisDate,	minDate[p],		maxDate[p],		chart->plotArea().width());
	adjustAxis(xAxisValue,	minRealYear[p],	maxRealYear[p],	chart->plotArea().width(),	rangeBufferFactorX, true);
	adjustAxis(yAxis,		0,				maxY[p],		chart->plotArea().height(),	rangeBufferFactorY);
	
	xAxisDate->setVisible(lowRange[p]);
	xAxisValue->setVisible(!lowRange[p]);
	
	for (QAbstractSeries* const series : chart->series()) {
		chart->removeSeries(series);
	}
	for (QXYSeries* const series : xySeries[p]) {
		chart->addSeries(series);
		if (lowRange[p])	series->attachAxis(xAxisDate);
		else				series->attachAxis(xAxisValue);
		series->attachAxis(yAxis);
	}
	
	chart->legend()->setVisible(xySeries[p].length() > 1);
	
	chartView->setUpdatesEnabled(true);
}

/**
 * Resets zoom level set by user.
 * 
 * To be called when user requests to reset the chart view.
 */
void TimeScatterChart::resetZoom()
{
	if (!hasData) return;
	chart->zoomReset();
}





/**
 * Creates a HistogramChart.
 * 
 * @param chartTitle		The title of the chart, to be displayed above it.
 * @param numClasses		The number of classes in the histogram.
 * @param classIncrement	The value increment between classes in the histogram.
 * @param classMax			The lower bound of the highest class in the histogram.
 * @param classNames		The list of translated class names for the x-axis.
 */
HistogramChart::HistogramChart(const QString& chartTitle, int numClasses, int classIncrement, int classMax, const QStringList& classNames) :
	Chart(chartTitle),
	numClasses		(numClasses),
	classIncrement	(classIncrement),
	classMax		(classMax),
	classNames		(classNames),
	xAxis		(nullptr),
	yAxis		(nullptr),
	barSeries	(nullptr),
	barSet		(nullptr),
	maxY	{0, 0}
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
	xAxis		= createBarCategoryXAxis(chart, Qt::AlignLeft, classNames);
	yAxis		= createValueYAxis(chart, QString(), Qt::AlignBottom);
	barSeries	= createHorizontalBarSeries(chart, xAxis, yAxis);
	barSet		= createBarSet(QString(), barSeries);
	chartView	= createChartView(chart, 52 + 26 * xAxis->count());
	
	connect(chartView, &SizeResponsiveChartView::wasResized, this, &HistogramChart::updateView);
}

/**
 * Clears displayed data from the chart.
 */
void HistogramChart::clear()
{
	barSet->remove(0, barSet->count());
	if (!usePinnedRanges) resetAxis(yAxis, true);
}

/**
 * Removes all data, resets pinned ranges and shows an empty chart.
 */
void HistogramChart::reset()
{
	barSet->remove(0, barSet->count());
	hasData = false;
	for (const bool p : {false, true}) {
		maxY[p] = 0;
	}
	resetAxis(yAxis, true);
}

/**
 * Assigns a class index to a given value.
 * 
 * @param value	The value to classify.
 * @return		An integer representing the class the given value belongs to.
 */
int HistogramChart::classifyValue(int value) const
{
	return std::min(value, classMax) / classIncrement;
}

/**
 * Replaces the displayed data and stores range information for future view updates.
 * 
 * Performs a view update before replacing the data.
 * 
 * @param histogramData		A list of data points to display in the chart. The length of the list must match the number of classes.
 * @param newMaxY			The maximum y value in the given data.
 * @param setPinnedRanges	Whether to store the given range information as pinned range data.
 */
void HistogramChart::updateData(QList<qreal> histogramData, qreal newMaxY, bool setPinnedRanges)
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
		clear();
		return;
	}
	assert(newMaxY > 0);
	
	for (int p = 0; p < (setPinnedRanges ? 2 : 1); p++) {
		maxY[p] = newMaxY;
	}
	
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
	
	const bool p = usePinnedRanges;
	adjustAxis(yAxis, 0, maxY[p], chart->plotArea().width(), rangeBufferFactorY);
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
	barSet		(nullptr),
	maxY	{0, 0}
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
 * Clears displayed data from the chart.
 */
void TopNChart::clear()
{
	barSet->remove(0, barSet->count());
	xAxis->setCategories({ "" });	// Workaround for QTBUG-122408: Category names not actually cleared properly by clear()
	xAxis->clear();
	resetAxis(yAxis, true);
}

/**
 * Removes all data, resets pinned ranges and shows an empty chart.
 */
void TopNChart::reset()
{
	barSet->remove(0, barSet->count());
	xAxis->setCategories({""});	// Workaround for QTBUG-122408: Category names not actually cleared properly by clear()
	xAxis->clear();
	hasData = false;
	for (const bool p : {false, true}) {
		maxY[p] = 0;
	}
	resetAxis(yAxis, true);
}

/**
 * Replaces the displayed data and stores range information for future view updates.
 * 
 * Performs a view update after replacing the data.
 * 
 * @param labels			The labels for the data points to display in the chart. The length of the list must be n or less.
 * @param values			The values for the data points to display in the chart. The length of the list must match the length of the labels list.
 * @param setPinnedRanges	Whether to store the given range information as pinned range data.
 */
void TopNChart::updateData(QStringList labels, QList<qreal> values, bool setPinnedRanges)
{
	assert(labels.size() == values.size());
	assert(labels.size() <= n);
	
	if (values.isEmpty()) {
		clear();
		return;
	}
	
	for (int p = 0; p < (setPinnedRanges ? 2 : 1); p++) {
		maxY[p] = values.first();
	}
	
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
	
	const bool p = usePinnedRanges;
	adjustAxis(yAxis, 0, maxY[p], chart->plotArea().width(), rangeBufferFactorY);
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
