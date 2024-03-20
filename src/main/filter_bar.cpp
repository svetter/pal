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
 * @file filter_bar.cpp
 * 
 * This file implements the FilterBar class.
 */

#include "filter_bar.h"

#include "main_window.h"
#include "src/filters/filter_widgets/id_filter_box.h"



/**
 * Creates a new filter bar.
 * 
 * @param parent	The parent widget.
 */
FilterBar::FilterBar(QWidget* parent) :
	QWidget(parent),
	mainWindow(nullptr),
	db(nullptr),
	compAscents(nullptr),
	temporarilyIgnoreChangeEvents(false),
	addFilterMenu(QMenu(this)),
	filterWizard(nullptr),
	filterBoxes(QList<FilterBox*>())
{
	setupUi(this);
	
	applyFiltersButton->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
	clearFiltersButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
	filtersScrollArea->setBackgroundRole(QPalette::Base);
	
	connect(applyFiltersButton,	&QPushButton::clicked,	this,	&FilterBar::handle_applyFilters);
	connect(clearFiltersButton,	&QPushButton::clicked,	this,	&FilterBar::handle_clearFilters);
}

FilterBar::~FilterBar()
{
	delete filterWizard;
	qDeleteAll(filterBoxes);
}

// INITIAL SETUP

/**
 * Supplies pointers to the main window, the database and the ascent table.
 * 
 * This function is called only once from the MainWindow constructor.
 * It exists so that the FilterBar can be placed in the UI form file, which means that its
 * constructor can't take any custom arguments like these pointers.
 * 
 * @param mainWindow	The main window.
 * @param db			The database.
 * @param compAscents	The ascent table.
 */
void FilterBar::supplyPointers(MainWindow* mainWindow, Database* db, CompositeAscentsTable* compAscents)
{
	this->mainWindow	= mainWindow;
	this->db			= db;
	this->compAscents	= compAscents;
	
	addFilterMenu.setTitle("New filter");
	for (const Column* const column : db->ascentsTable.getColumnList()) {
		addFilterMenu.addAction(column->uiName);
	}
	addFilterButton->setMenu(&addFilterMenu);
	
	filterWizard = new FilterWizard(this, db->ascentsTable);
	connect(addFilterButton,	&QToolButton::clicked,		this,	&FilterBar::handle_newFilterButtonPressed);
	connect(filterWizard,		&FilterWizard::accepted,	this,	&FilterBar::handle_newFilterCreated);
	
	resetUI();
}



// PROJECT SETUP

/**
 * Resets all UI elements to their default state.
 * 
 * Project-specific combo boxes stay populated.
 */
void FilterBar::resetUI()
{
	// TODO
}

/**
 * Sets UI up to represent the given set of active filters.
 * 
 * @param filters	The set of active filters to represent in the UI.
 */
void FilterBar::insertFiltersIntoUI(QSet<Filter*> filters)
{
	resetUI();
	
	if (filters.isEmpty()) return;
	
	for (const Filter* const filter : filters) {
		// TODO
	}
	
	handle_filtersChanged();
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
}



// UPDATE UI

void FilterBar::updateIDCombos()
{
	for (FilterBox* const filterBox : filterBoxes) {
		if (filterBox->type == ID) {
			IDFilterBox* idFilterBox = (IDFilterBox*) filterBox;
			idFilterBox->setComboEntries();
		}
	}
}



// UI CHANGE HANDLERS

/**
 * Collective event handler for any changes to filter UI elements.
 * 
 * Sets 'Apply filters' button to enabled if any filter is enabled in the UI and to disabled
 * otherwise.
 * Sets 'Clear filters' button to enabled if any filters are currently applied to the table (Note
 * that this is completely independent of the filter UI).
 */
void FilterBar::handle_filtersChanged()
{
	if (temporarilyIgnoreChangeEvents) return;
	
	// TODO
}



// EXECUTE FILTER ACTIONS

void FilterBar::handle_newFilterButtonPressed()
{
	assert(filterWizard);
	
	filterWizard->restart();
	filterWizard->show();
}

void FilterBar::handle_newFilterCreated()
{
	assert(filterWizard);
	
	unique_ptr<Filter> newFilter = filterWizard->getFinishedFilter();
	FilterBox* newFilterBox = newFilter->getFilterBox(filtersScrollAreaWidget);
	filterBoxes.append(newFilterBox);
	filtersScrollAreaLayout->addWidget(newFilterBox);
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
	
	connect(newFilterBox, &FilterBox::removeRequested, this, &FilterBar::handle_removeFilter);
}

void FilterBar::handle_removeFilter()
{
	FilterBox* const filterBox = (FilterBox*) sender();
	filtersScrollAreaLayout->removeWidget(filterBox);
	filterBoxes.removeAll(filterBox);
	delete filterBox;
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
}



// EXECUTE FILTER ACTIONS

/**
 * Event handler for the 'Apply filters' button.
 * 
 * Updates enabled status of 'Apply filters' and 'Clear filters' buttons, collects, saves and
 * applies the filters specified in the UI, and prompts the main window to update the table size
 * information.
 */
void FilterBar::handle_applyFilters()
{
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
	
	QSet<Filter*> filters = collectFilters();
	compAscents->applyFilters(filters);
	saveFilters(filters);
	
	mainWindow->currentFiltersChanged();
}

/**
 * Event handler for the 'Clear filters' button.
 * 
 * Sets the 'Clear filters' button to disabled, and performs the steps necessary to clear the
 * filters from the table and update the table size information.
 */
void FilterBar::handle_clearFilters()
{
	compAscents->clearFilters();
	
	handle_filtersChanged();	// Potentially enable apply button
	
	mainWindow->currentFiltersChanged();
}



// PARSING FILTERS FROM UI

/**
 * Assembles the set of active filters specified in the UI.
 * 
 * Any filter whose checkbox is enabled is included in the set, and vice versa.
 * 
 * @return	A set representing the active filters currently specified in the UI.
 */
QSet<Filter*> FilterBar::collectFilters()
{
	QSet<Filter*> filters = QSet<Filter*>();
	
	// TODO
	
	return filters;
}



// SAVING FILTERS

/**
 * Saves the given set of filters to the project settings.
 * 
 * Any filter not included in the set will be cleared from the project settings.
 * 
 * @param filters	The set of filters to save.
 */
void FilterBar::saveFilters(const QSet<Filter*> filters)
{
	// TODO
	
	for (const Filter* const filter : filters) {
		// TODO
	}
}



// RETRIEVING FILTERS FROM PROJECT SETTINGS

/**
 * Retrieves the set of filters saved in the project settings.
 * 
 * Filters not set in the project settings will not be included in the set.
 * 
 * @return	A set of filters representing the ones saved in the project settings.
 */
QSet<Filter*> FilterBar::parseFiltersFromProjectSettings()
{
	QSet<Filter*> filters = QSet<Filter*>();
	
	ProjectSettings& settings = db->projectSettings;
	
	// TODO
	
	return filters;
}
