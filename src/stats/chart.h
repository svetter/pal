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
 * @file chart.h
 * 
 * This file declares the Chart class and its subclasses.
 */

#ifndef CHART_H
#define CHART_H

#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QHorizontalBarSeries>
#include <QLineSeries>
#include <QScatterSeries>
#include <QDateTime>



/**
 * A QChartView subclass which forwards resize events, allowing them to be used externally.
 */
class SizeResponsiveChartView : public QChartView
{
	Q_OBJECT
	
public:
	/**
	 * Creates a new SizeResponsiveChartView.
	 * 
	 * Delegates to QChartView constructor.
	 * 
	 * @param chart	The QChart object to create a view of.
	 */
	inline SizeResponsiveChartView(QChart* chart) :
		QChartView(chart)
	{}
	
	/**
	 * Event handler for resize events.
	 * 
	 * Calls the inherited event handler for the same purpose, then emits wasResized().
	 */
	virtual void inline resizeEvent(QResizeEvent* event) override
	{
		QChartView::resizeEvent(event);
		Q_EMIT wasResized();
	}
	
	/**
	 * Event handler for mouse double-click events.
	 * 
	 * Calls the inherited event handler for the same purpose, then emits receivedDoubleClick().
	 */
	virtual void inline mouseDoubleClickEvent(QMouseEvent* event) override
	{
		QChartView::mouseDoubleClickEvent(event);
		Q_EMIT receivedDoubleClick();
	}
	
signals:
	/**
	 * Signal function which is called whenever the chart view has been resized.
	 */
	void wasResized();
	
	/**
	 * Signal function which is called whenever the user double-clicks the chart view.
	 */
	void receivedDoubleClick();
};



/**
 * A class which encapsules all functionality used for a single statistical chart in the UI.
 * 
 * Subclasses define details specific to certain types of charts, e.g. what kind of axes to use.
 */
class Chart : public QObject
{
protected:
	/** Constant controlling the targeted gap in pixels between two label ticks on a chart axis. */
	inline static const int		pixelsPerTick		= 70;
	/** Constant controlling how wide to make the range buffer on each side of an x-axis as a fraction of the total range. */
	inline static const qreal	rangeBufferFactorX	= 0.01;
	/** Constant controlling how wide to make the range buffer on each side of a y-axis as a fraction of the total range. */
	inline static const qreal	rangeBufferFactorY	= 0.025;
	
	/** The title of the chart, to be displayed above it. */
	const QString chartTitle;
	
	/** The QChart object for the chart. */
	QChart* chart;
	/** The chart view object for the chart. */
	SizeResponsiveChartView* chartView;
	/** Indicates whether the chart is currently displaying any data and that range values have been set. */
	bool hasData;
	
public:
	Chart(const QString& chartTitle);
	virtual ~Chart();
	
	/**
	 * Performs the setup for the chart.
	 */
	virtual void setup() = 0;
	
	/**
	 * Removes all data from the chart.
	 */
	virtual void reset() = 0;
	
	/**
	 * Updates the chart layout, e.g. tick spacing, without changing the displayed data.
	 */
	virtual void updateView() = 0;
	
	QChartView* getChartView() const;
	
protected:
	// Setup helpers
	static QChart* createChart(const QString& title);
	static QValueAxis* createValueXAxis(QChart* chart, const QString& title = QString());
	static QDateTimeAxis* createDateXAxis(QChart* chart, const QString& title = QString());
	static QBarCategoryAxis* createBarCategoryXAxis(QChart* chart, const Qt::AlignmentFlag alignment = Qt::AlignBottom, const QStringList& categories = {});
	static QValueAxis* createValueYAxis(QChart* chart, const QString& title = QString(), const Qt::AlignmentFlag alignment = Qt::AlignLeft);
	static SizeResponsiveChartView* createChartView(QChart* chart, int minimumHeight = -1);
	
	static QBarSeries* createBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis);
	static QHorizontalBarSeries* createHorizontalBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis);
	static QBarSet* createBarSet(const QString& name, QAbstractBarSeries* series);
	
	// Update helpers
public:
	static QLineSeries* createLineSeries(const QString& name = QString());
	static QScatterSeries* createScatterSeries(const QString& name, int markerSize = -1, QScatterSeries::MarkerShape markerShape = QScatterSeries::MarkerShape(-1));
protected:
	static void adjustAxis(QValueAxis* axis, qreal minValue, qreal maxValue, int chartSize, qreal rangeBufferFactor = 0, bool isTimeAxis = false);
	static void adjustAxis(QDateTimeAxis* axis, QDate minValue, QDate maxValue, int chartSize);
	static void resetAxis(QValueAxis* axis, bool show0Tick);
	static void resetAxis(QDateTimeAxis* axis);
};



/**
 * A class representing a bar chart with a horizontal x-axis which shows years, and a vertical real-
 * number y-axis.
 */
class YearBarChart : public Chart
{
protected:
	/** The translated label for the chart's y-axis. */
	const QString yAxisTitle;
	
	/** The x-axis for the chart. */
	QValueAxis*	xAxis;
	/** The y-axis for the chart. */
	QValueAxis*	yAxis;
	/** The bar series for the chart. A bar series represents a set of bars, each of which belongs to another category on the x-axis, but all of which share the same label. */
	QBarSeries*	barSeries;
	/** The bar set for the chart. A bar set contains one data value for each x-axis category. */
	QBarSet*	barSet;
	
	// Range data
	/** The minimum x value of the current data set. */
	int minYear;
	/** The maximum x value of the current data set. */
	int maxYear;
	/** The maximum y value of the current data set. */
	qreal maxY;
	
public:
	YearBarChart(const QString& chartTitle, const QString& yAxisTitle = QString());
	virtual ~YearBarChart();
	
	virtual void setup() override;
	virtual void reset() override;
	void updateData(const QList<qreal>& newData, int minYear, int maxYear, qreal maxY);
	virtual void updateView() override;
};



/**
 * A class containing raw data and configuration for a single series in a date-based scatterplot.
 */
class DateScatterSeries {
public:
	/** The translated label for the series. */
	QString							name;
	/** The size of the markers for the series. */
	int								markerSize;
	/** The shape of the markers for the series. */
	QScatterSeries::MarkerShape		markerShape;
	/** The data points for the series, consisting of a date-time and a real number. */
	QList<QPair<QDateTime, qreal>>	data;
	
	DateScatterSeries(const QString& name, int markerSize, QScatterSeries::MarkerShape markerShape);

	QScatterSeries* createScatterSeries() const;
};



/**
 * A class representing a scatterplot with a horizontal x-axis which shows time, and a vertical
 * real-number y-axis.
 */
class TimeScatterChart : public Chart
{
protected:
	/** The translated label for the chart's y-axis. */
	const QString yAxisTitle;
	
	/** The date-based x-axis for the chart. */
	QDateTimeAxis*	xAxisDate;
	/** The real-value-based x-axis for the chart. */
	QValueAxis*		xAxisValue;
	/** The y-axis for the chart. */
	QValueAxis*		yAxis;
	
	// Range data
	/** Indicates whether the current data sets have a range below the threshold, where a date-based x-axis is used. */
	bool lowRange;
	/** The minimum date of the current data sets. */
	QDate minDate;
	/** The maximum date of the current data sets. */
	QDate maxDate;
	/** The minimum real-value representaition of the minimum date (x-value) of the current data sets. */
	qreal minRealYear;
	/** The maximum real-value representaition of the minimum date (x-value) of the current data sets. */
	qreal maxRealYear;
	/** The maximum y value of the current data sets. */
	qreal maxY;
	
public:
	TimeScatterChart(const QString& chartTitle, const QString& yAxisTitle = QString());
	virtual ~TimeScatterChart();
	
	virtual void setup() override;
	virtual void reset() override;
	void updateData(const QList<const DateScatterSeries*>& seriesData, QDate minDate, QDate maxDate, qreal maxY);
	virtual void updateView() override;
	void resetZoom();
};



/**
 * A class representing a bar graph with a vertical x-axis which contains categories, and a
 * horizontal real-number y-axis.
 */
class HistogramChart : public Chart
{
protected:
	/** The list of translated category names for the x-axis. */
	const QStringList xAxisCategories;
	
	/** The x-axis for the chart. */
	QBarCategoryAxis*		xAxis;
	/** The y-axis for the chart. */
	QValueAxis*				yAxis;
	/** The bar series for the chart. A bar series represents a set of bars, each of which belongs to another category on the x-axis, but all of which share the same label. */
	QHorizontalBarSeries*	barSeries;
	/** The bar set for the chart. A bar set contains one data value for each x-axis category. */
	QBarSet*				barSet;
	
	// Range data
	/** The maximum y value of the current data sets. */
	qreal maxY;
	
public:
	HistogramChart(const QString& chartTitle, const QStringList& xAxisCategories);
	virtual ~HistogramChart();
	
	virtual void setup() override;
	virtual void reset() override;
	void updateData(QList<qreal> histogramData, qreal maxY);
	virtual void updateView() override;
};



/**
 * A class representing a horizontal bar graph showing the n items currently in a filtered table
 * with the highest value in some metric.
 */
class TopNChart : public Chart
{
public:
	/** The number of items to show. */
	const int n;
protected:
	/** The translated label for the chart's y-axis. */
	const QString yAxisTitle;
	
	/** The x-axis for the chart. */
	QBarCategoryAxis*		xAxis;
	/** The y-axis for the chart. */
	QValueAxis*				yAxis;
	/** The bar series for the chart. A bar series represents a set of bars, each of which belongs to another category on the x-axis, but all of which share the same label. */
	QHorizontalBarSeries*	barSeries;
	/** The bar set for the chart. A bar set contains one data value for each x-axis category. */
	QBarSet*				barSet;
	
	// Range data
	/** The maximum y value of the current data set. */
	qreal maxY;
	
public:
	TopNChart(int n, const QString& chartTitle, const QString& yAxisTitle = QString());
	virtual ~TopNChart();
	
	virtual void setup() override;
	virtual void reset() override;
	void updateData(QStringList labels, QList<qreal> values);
	virtual void updateView() override;
	
private:
	static void renameDuplicates(QStringList& list);
};



#endif // CHART_H
