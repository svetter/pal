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
 * This file declares the GeneralStatsEngine class.
 */

#ifndef STATS_ENGINE_H
#define STATS_ENGINE_H

#include "src/db/database.h"

#include <QLayout>
#include <QChart>
#include <QValueAxis>
#include <QChartView>
#include <QLineSeries>
#include <QScatterSeries>



class GeneralStatsEngine
{
	Database* const db;
	
	QVBoxLayout** const statisticsTabLayoutPtr;
	
	QChart* elevGainPerYearChart;
	QValueAxis* elevGainPerYearXAxis;
	QValueAxis* elevGainPerYearYAxis;
	QChart* numAscentsPerYearChart;
	QValueAxis* numAscentsPerYearXAxis;
	QValueAxis* numAscentsPerYearYAxis;
	QChart* heightsScatterChart;
	QValueAxis* heightsScatterXAxis;
	QValueAxis* heightsScatterYAxis;
	
public:
	GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr);
	
	void setupStatsTab();
	void updateStatsTab();
	
private:
	// Setup helpers
	static QChart* createChart(const QString& title, bool displayLegend);
	static QValueAxis* createXAxis(QChart* chart, const QString& title = QString());
	static QValueAxis* createYAxis(QChart* chart, const QString& title = QString());
	static QChartView* createChartView(QChart* chart);
	static void addChartsToLayout(QBoxLayout* layout, const QList<QChartView*>& charts, QList<int> stretchFactors = QList<int>());
	
	// Update helpers
	static QLineSeries* createLineSeries(const QString& name);
	static QScatterSeries* createScatterSeries(const QString& name, int markerSize = -1, QScatterSeries::MarkerShape marker = QScatterSeries::MarkerShape(-1));
	static void updateSeriesForChartWithYearXAxis(QChart* chart, QValueAxis* xAxis, QValueAxis* yAxis, const QList<QXYSeries*>& newSeries, qreal minYear, qreal maxYear, int yearInterval);
};



#endif // STATS_ENGINE_H
