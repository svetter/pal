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
 * @file stats_engine.h
 * 
 * This file declares the StatsEngine, GeneralStatsEngine and ItemStatsEngine classes.
 */

#ifndef STATS_ENGINE_H
#define STATS_ENGINE_H

#include "src/db/database.h"

#include <QLayout>
#include <QChart>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QChartView>
#include <QLineSeries>
#include <QScatterSeries>
#include <QBarSet>
#include <QAbstractBarSeries>
#include <QHorizontalBarSeries>



class StatsEngine
{
protected:
	Database* const db;
	
	inline static const int pixelsPerTick = 100;
	
	StatsEngine(Database* db);
	
	// Setup helpers
	static QChart* createChart(const QString& title, bool displayLegend);
	static QValueAxis* createValueXAxis(QChart* chart, const QString& title = QString());
	static QBarCategoryAxis* createBarCategoryXAxis(QChart* chart, const QStringList& categories, const Qt::AlignmentFlag alignment = Qt::AlignBottom);
	static QValueAxis* createValueYAxis(QChart* chart, const QString& title = QString(), const Qt::AlignmentFlag alignment = Qt::AlignLeft);
	static QChartView* createChartView(QChart* chart, int minimumHeight = -1);
	static void addChartsToLayout(QBoxLayout* layout, const QList<QChartView*>& charts, QList<int> stretchFactors = QList<int>());
	
	static QHorizontalBarSeries* createHorizontalBarSeries(QChart* chart, QAbstractAxis* xAxis, QAbstractAxis* yAxis);
	static QBarSet* createBarSet(const QString& name, QAbstractBarSeries* series);
	
	// Update helpers
	static QLineSeries* createLineSeries(const QString& name);
	static QScatterSeries* createScatterSeries(const QString& name, int markerSize = -1, QScatterSeries::MarkerShape marker = QScatterSeries::MarkerShape(-1));
	static void updateSeriesForChartWithYearXAxis(QChart* chart, QValueAxis* xAxis, QValueAxis* yAxis, const QList<QXYSeries*>& newSeries, qreal minYear, qreal maxYear, qreal minY, qreal maxY, bool bufferXAxisRange);
	static void adjustAxis(QValueAxis* axis, qreal minValue, qreal maxValue, int chartSize, qreal rangeBufferFactor = 0);
	static qreal findTickIntervalAndMinorTickCount(int range, int chartSize);
};



class GeneralStatsEngine : public StatsEngine
{
	QVBoxLayout** const statisticsTabLayoutPtr;
	
	QChart*		elevGainPerYearChart;
	QValueAxis*	elevGainPerYearXAxis;
	QValueAxis*	elevGainPerYearYAxis;
	QChartView*	elevGainPerYearChartView;
	QChart*		numAscentsPerYearChart;
	QValueAxis*	numAscentsPerYearXAxis;
	QValueAxis*	numAscentsPerYearYAxis;
	QChartView*	numAscentsPerYearChartView;
	QChart*		heightsScatterChart;
	QValueAxis*	heightsScatterXAxis;
	QValueAxis*	heightsScatterYAxis;
	QChartView*	heightsScatterChartView;
	
public:
	GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr);
	
	void setupStatsTab();
	void updateStatsTab();
};



class ItemStatsEngine : public StatsEngine
{
	QVBoxLayout* const statsFrameLayout;
	
	QChart*		peakHeightHistChart;
	QBarSet*	peakHeightHistBarSet;
	QValueAxis*	peakHeightHistYAxis;
	QChartView*	peakHeightHistChartView;
	QChart*		elevGainHistChart;
	QBarSet*	elevGainHistBarSet;
	QValueAxis*	elevGainHistYAxis;
	QChartView*	elevGainHistChartView;
	
public:
	ItemStatsEngine(Database* db, QVBoxLayout* statsFrameLayout);
	
	void setupStatsPanel();
	void updateStatsPanel(const QSet<BufferRowIndex>& selectedBufferRows);
};



#endif // STATS_ENGINE_H
