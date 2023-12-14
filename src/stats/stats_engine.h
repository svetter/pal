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

#include "chart.h"
#include "src/data/item_types.h"
#include "src/comp_tables/breadcrumbs.h"

#include <QBoxLayout>



class StatsEngine
{
protected:
	Database* const db;
	
	StatsEngine(Database* db);
	virtual ~StatsEngine();
	
	static void addChartsToLayout(QBoxLayout* layout, const QList<QChartView*>& charts, QList<int> stretchFactors = QList<int>());
};



class GeneralStatsEngine : public StatsEngine
{
	QVBoxLayout** const statisticsTabLayoutPtr;
	
	YearChart* elevGainPerYearChart;
	YearChart* numAscentsPerYearChart;
	YearChart* heightsScatterChart;
	
public:
	GeneralStatsEngine(Database* db, QVBoxLayout** const statisticsTabLayoutPtr);
	virtual ~GeneralStatsEngine();
	
	void setupStatsTab();
	void updateStatsTab();
};



class ItemStatsEngine : public StatsEngine
{
	const PALItemType itemType;
	QVBoxLayout* const statsFrameLayout;
	
	HistogramChart* peakHeightHistChart;
	HistogramChart* elevGainHistChart;
	
public:
	ItemStatsEngine(Database* db, PALItemType itemType, QVBoxLayout* statsFrameLayout);
	virtual ~ItemStatsEngine();
	
	void setupStatsPanel();
	void updateStatsPanel(const QSet<BufferRowIndex>& selectedBufferRows);
	
private:
	Breadcrumbs getBreadcrumbsFor(NormalTable* destinationTable);
};



#endif // STATS_ENGINE_H
