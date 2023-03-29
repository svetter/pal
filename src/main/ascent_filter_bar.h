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

#ifndef ASCENT_FILTER_BAR_H
#define ASCENT_FILTER_BAR_H

#include "src/comp_tables/filter.h"
#include "src/db/database.h"
#include "ui_ascent_filter_bar.h"

#include <QMainWindow>
#include <QTableView>

class MainWindow;



class AscentFilterBar : public QWidget, public Ui_AscentFilterBar
{
	Q_OBJECT
	
	MainWindow* mainWindow;
	Database* db;
	CompositeAscentsTable* compAscents;
	
	QList<ValidItemID> selectableRangeIDs;
	QList<ValidItemID> selectableHikerIDs;
	
public:
	AscentFilterBar(QWidget* parent);
	// Initial setup
	void supplyPointers(MainWindow* mainWindow, Database* db, CompositeAscentsTable* compAscents);
private:
	void connectUI();
	void additionalUISetup();
	
public:
	// Project setup
	void resetUI();
	void insertFiltersIntoUI(QSet<Filter> filters);
	
public:
	// Update UI
	void updateRangeCombo();
	void updateHikerCombo();
	
private slots:
	// UI change handlers
	void handle_filtersChanged();
	void handle_difficultyFilterBoxChanged();
	void handle_minDateChanged();
	void handle_maxDateChanged();
	void handle_minHeightChanged();
	void handle_maxHeightChanged();
	void handle_difficultyFilterSystemChanged();
	
public:
	// Execute filter actions
	void handle_applyFilters();
	void handle_clearFilters();
	
private:
	// Parsing filters from UI
	QSet<Filter> collectFilters();
	
	// Saving filters
	void clearSavedFilters();
	void saveFilters(const QSet<Filter> filters);
	
public:
	// Retrieving filters from project settings
	QSet<Filter> parseFiltersFromProjectSettings();
};



#endif // ASCENT_FILTER_BAR_H
