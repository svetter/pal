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
 * @file chart.h
 * 
 * This file declares the chart class and its subclasses.
 */

#ifndef CHART_H
#define CHART_H

#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QHorizontalBarSeries>
#include <QLineSeries>
#include <QScatterSeries>



class SizeResponsiveChartView : public QChartView
{
	Q_OBJECT
	
public:
	inline SizeResponsiveChartView(QChart* chart) :
		QChartView(chart)
	{}
	
	virtual void inline resizeEvent(QResizeEvent* event) override
	{
		QChartView::resizeEvent(event);
		Q_EMIT wasResized();
	}
signals:
	void wasResized();
};



class Chart : public QObject
{
protected:
	inline static const int pixelsPerTick = 100;
	inline static const qreal rangeBufferFactorX = 0.01;
	inline static const qreal rangeBufferFactorY = 0.05;
	
	const QString chartTitle;
	
	QChart* chart;
	SizeResponsiveChartView* chartView;
	bool hasData;
	
public:
	Chart(const QString& chartTitle);
	virtual ~Chart();
	
	virtual void setup() = 0;
	virtual void updateView() = 0;
	
	QChartView* getChartView() const;
	
protected:
	// Setup helpers
	static QChart* createChart(const QString& title);
	static QValueAxis* createValueXAxis(QChart* chart, const QString& title = QString());
	static QBarCategoryAxis* createBarCategoryXAxis(QChart* chart, const QStringList& categories, const Qt::AlignmentFlag alignment = Qt::AlignBottom);
	static QValueAxis* createValueYAxis(QChart* chart, const QString& title = QString(), const Qt::AlignmentFlag alignment = Qt::AlignLeft);
	static SizeResponsiveChartView* createChartView(QChart* chart, int minimumHeight = -1);
	
	static QHorizontalBarSeries* createHorizontalBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis);
	static QBarSet* createBarSet(const QString& name, QAbstractBarSeries* series);
	
	// Update helpers
public:
	static QLineSeries* createLineSeries(const QString& name);
	static QScatterSeries* createScatterSeries(const QString& name, int markerSize = -1, QScatterSeries::MarkerShape marker = QScatterSeries::MarkerShape(-1));
protected:
	static void adjustAxis(QValueAxis* axis, qreal minValue, qreal maxValue, int chartSize, qreal rangeBufferFactor = 0);
};



class YearChart : public Chart
{
protected:
	const QString yAxisTitle;
	const bool bufferXAxisRange;
	
	QValueAxis*	xAxis;
	QValueAxis*	yAxis;
	
	// Range data
	int minYear;
	int maxYear;
	qreal minY;
	qreal maxY;
	
public:
	YearChart(const QString& chartTitle, const QString& yAxisTitle, bool bufferXAxisRange);
	virtual ~YearChart();
	
	virtual void setup() override;
	void updateData(const QList<QXYSeries*>& newSeries, qreal minYear, qreal maxYear, qreal minY, qreal maxY);
	virtual void updateView() override;
};



class HistogramChart : public Chart
{
protected:
	const QStringList xAxisCategories;
	const QString barSetTitle;
	
	QBarCategoryAxis*		xAxis;
	QValueAxis*				yAxis;
	QHorizontalBarSeries*	barSeries;
	QBarSet*				barSet;
	
	// Range data
	qreal maxY;
	
public:
	HistogramChart(const QString& chartTitle, const QStringList& xAxisCategories, const QString& barSetTitle);
	virtual ~HistogramChart();
	
	virtual void setup() override;
	void updateData(QList<qreal> histogramData, qreal maxY);
	virtual void updateView() override;
};



#endif // CHART_H
