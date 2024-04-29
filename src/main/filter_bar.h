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
 * @file filter_bar.h
 * 
 * This file declares the FilterBar class.
 */

#ifndef FILTER_BAR_H
#define FILTER_BAR_H

#include "src/comp_tables/composite_table.h"
#include "src/filters/filter.h"
#include "src/db/database.h"
#include "src/filters/filter_widgets/filter_box.h"
#include "src/main/filter_wizard.h"
#include "ui_filter_bar.h"

#include <QMainWindow>
#include <QTableView>
#include <QMenu>

class MainWindow;
class ItemTypeMapper;



/**
 * Control class for the filter bar.
 */
class FilterBar : public QWidget, public Ui_FilterBar
{
	Q_OBJECT
	
	/** The main window containing the composite table and filter bar. */
	MainWindow* mainWindow;
	/** The project's database. */
	Database* db;
	/** The composite table. */
	CompositeTable* compTable;
	ItemTypeMapper* mapper;
	
	QList<FilterBox*> filterBoxes;
	
	QMenu quickFilterMenu;
	QHash<QAction*, const CompositeColumn*> quickFilterActions;
	FilterWizard* filterWizard;
	
public:
	FilterBar(QWidget* parent);
	~FilterBar();
	// Initial setup
	void supplyPointers(MainWindow* mainWindow, Database* db, ItemTypeMapper* mapper);
	
public:
	// Project setup
	void resetUI();
	void insertFiltersIntoUI(const QList<Filter*>& filters);
	
	// Update UI
private:
	void updateQuickFilterMenu();
public:
	void updateIDCombos();
	
	// Remove filters
	void removeFilter(FilterBox* filterBox);
	void compColumnAboutToBeRemoved(const CompositeColumn& column);
	
private slots:
	// UI change handlers
	void handle_filtersChanged();
	
public:
	// Execute filter actions
	void handle_newFilterButtonPressed();
	void handle_filterWizardAccepted();
	void handle_quickFilterActionUsed();
	void handle_removeFilter();
	void handle_applyFilters();
	void handle_clearFilters();
	
private:
	// Parsing filters from UI
	QList<const Filter*> collectAllFilters();
	QList<const Filter*> collectEnabledFilters();
	
public:
	// Saving filters
	void saveFilters();
	// Retrieving filters from project settings
	QList<Filter*> parseFiltersFromProjectSettings(const ItemTypesHandler &typesHandler);
};



#endif // FILTER_BAR_H
