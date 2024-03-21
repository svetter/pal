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
#include "src/filters/bool_filter.h"
#include "src/filters/date_filter.h"
#include "src/filters/dual_enum_filter.h"
#include "src/filters/enum_filter.h"
#include "src/filters/id_filter.h"
#include "src/filters/int_filter.h"
#include "src/filters/string_filter.h"
#include "src/filters/time_filter.h"



/**
 * Creates a new filter bar.
 * 
 * @param parent	The parent widget.
 */
FilterBar::FilterBar(QWidget* parent) :
	QWidget(parent),
	mainWindow(nullptr),
	db(nullptr),
	compTable(nullptr),
	filterBoxes(QList<FilterBox*>()),
	addFilterMenu(QMenu(this)),
	filterWizard(nullptr)
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
 * Supplies pointers to the main window, the database and the composite table.
 * 
 * This function is called only once from the MainWindow constructor.
 * It exists so that the FilterBar can be placed in the UI form file, which means that its
 * constructor can't take any custom arguments like these pointers.
 * 
 * @param mainWindow	The main window.
 * @param db			The database.
 * @param compTable		The composite table.
 */
void FilterBar::supplyPointers(MainWindow* mainWindow, Database* db, CompositeTable* compTable)
{
	this->mainWindow	= mainWindow;
	this->db			= db;
	this->compTable		= compTable;


	// Create filter wizard
	filterWizard = new FilterWizard(this, compTable->getBaseTable());
	connect(addFilterButton, &QToolButton::clicked, this, &FilterBar::handle_newFilterButtonPressed);
	connect(filterWizard, &FilterWizard::accepted, this, &FilterBar::handle_filterWizardAccepted);

	// Create filter shortcuts
	addFilterMenu.setTitle("New filter");
	
	createFilterShortcuts.clear();
	for (const Column* const column : compTable->getBaseTable().getColumnList()) {
		// Create name
		QString name = QString();
		if (column->foreignColumn) {
			const Table& foreignTable = column->getReferencedForeignColumn().table;
			assert(!foreignTable.isAssociative);
			const NormalTable& targetTable = (const NormalTable&) foreignTable;
			name = targetTable.getItemNameSingular();
		} else {
			name = column->uiName;
		}
		
		if (column->type == DualEnum) {
			const bool isLastColumn = column->getIndex() == column->table.getNumberOfColumns() - 1;
			if (isLastColumn) continue;
			const Column& nextColumn = column->table.getColumnByIndex(column->getIndex() + 1);
			const bool belongsToNextColumn = nextColumn.enumNameLists == column->enumNameLists;
			if (belongsToNextColumn) {
				name += "/" + nextColumn.uiName;
			} else {
				continue;
			}
		}
		
		// Create action
		QAction* action = addFilterMenu.addAction(name);
		
		createFilterShortcuts.insert(action, column);
		
		connect(action, &QAction::triggered, this, &FilterBar::handle_filterCreationShortcutUsed);
	}
	addFilterButton->setMenu(&addFilterMenu);
	
	resetUI();
}



// PROJECT SETUP

/**
 * Resets all UI elements to their default state.
 */
void FilterBar::resetUI()
{
	while (!filterBoxes.isEmpty()) {
		emit filterBoxes.first()->removeRequested();
	}
}

/**
 * Sets UI up to represent the given set of active filters.
 * 
 * @param filters	The set of active filters to represent in the UI.
 */
void FilterBar::insertFiltersIntoUI(const QSet<const Filter*>& filters)
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
	for (const FilterBox* const filterBox : filterBoxes) {
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
	bool anyFilterEnabled = false;
	for (const FilterBox* const filterBox : filterBoxes) {
		anyFilterEnabled |= filterBox->isChecked();
	}
	applyFiltersButton->setEnabled(anyFilterEnabled);
	
	clearFiltersButton->setEnabled(false); // TODO
}



// EXECUTE FILTER ACTIONS

void FilterBar::handle_newFilterButtonPressed()
{
	assert(filterWizard);
	
	filterWizard->restart();
	filterWizard->show();
}

void FilterBar::handle_filterWizardAccepted()
{
	assert(filterWizard);
	
	unique_ptr<Filter> newFilter = filterWizard->getFinishedFilter();
	FilterBox* newFilterBox = newFilter->getFilterBox(filtersScrollAreaWidget, std::move(newFilter)).release();
	filterBoxes.append(newFilterBox);
	filtersScrollAreaLayout->addWidget(newFilterBox);
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
	
	connect(newFilterBox, &FilterBox::filterChanged,	this, &FilterBar::handle_filtersChanged);
	connect(newFilterBox, &FilterBox::removeRequested,	this, &FilterBar::handle_removeFilter);
	
	handle_filtersChanged();
}

void FilterBar::handle_filterCreationShortcutUsed()
{
	QAction* action = (QAction*) sender();
	assert(action);
	const QString name = action->text();
	const Column* const columnToUse = createFilterShortcuts.value(action);
	assert(columnToUse);
	const NormalTable& tableToFilter = compTable->getBaseTable();
	
	unique_ptr<Filter> newFilter = nullptr;
	switch (columnToUse->type) {
	case Integer:	newFilter = make_unique<IntFilter>		(tableToFilter, *columnToUse, name);	break;
	case ID:		newFilter = make_unique<IDFilter>		(tableToFilter, *columnToUse, name);	break;
	case Enum:		newFilter = make_unique<EnumFilter>		(tableToFilter, *columnToUse, name);	break;
	case DualEnum:	newFilter = make_unique<DualEnumFilter>	(tableToFilter, *columnToUse, name);	break;
	case Bit:		newFilter = make_unique<BoolFilter>		(tableToFilter, *columnToUse, name);	break;
	case String:	newFilter = make_unique<StringFilter>	(tableToFilter, *columnToUse, name);	break;
	case Date:		newFilter = make_unique<DateFilter>		(tableToFilter, *columnToUse, name);	break;
	case Time:		newFilter = make_unique<TimeFilter>		(tableToFilter, *columnToUse, name);	break;
	default: assert(false);
	}
	
	FilterBox* newFilterBox = newFilter->getFilterBox(filtersScrollAreaWidget, std::move(newFilter)).release();
	filterBoxes.append(newFilterBox);
	filtersScrollAreaLayout->addWidget(newFilterBox);
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
	
	connect(newFilterBox, &FilterBox::filterChanged,	this, &FilterBar::handle_filtersChanged);
	connect(newFilterBox, &FilterBox::removeRequested,	this, &FilterBar::handle_removeFilter);
	
	handle_filtersChanged();
}

void FilterBar::handle_removeFilter()
{
	FilterBox* const filterBox = (FilterBox*) sender();
	filtersScrollAreaLayout->removeWidget(filterBox);
	filterBoxes.removeAll(filterBox);
	delete filterBox;
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
	
	handle_filtersChanged();
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
	
	QSet<const Filter*> filters = collectFilters();
	compTable->applyFilters(filters);
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
	compTable->clearFilters();
	
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
QSet<const Filter*> FilterBar::collectFilters()
{
	QSet<const Filter*> filters = QSet<const Filter*>();
	
	for (const FilterBox* const filterBox : filterBoxes) {
		filterBox->getFilter();
	}
	
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
void FilterBar::saveFilters(const QSet<const Filter*>& filters)
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
QSet<const Filter*> FilterBar::parseFiltersFromProjectSettings()
{
	QSet<const Filter*> filters = QSet<const Filter*>();
	
	ProjectSettings& settings = db->projectSettings;
	
	// TODO
	
	return filters;
}
