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
 * @param mapper		The item type mapper for the table connected to the filter bar.
 */
void FilterBar::supplyPointers(MainWindow* mainWindow, Database* db, ItemTypeMapper* mapper)
{
	this->mainWindow	= mainWindow;
	this->db			= db;
	this->compTable		= &mapper->compTable;
	this->mapper		= mapper;
	
	
	// Create filter wizard
	filterWizard = new FilterWizard(this, compTable->baseTable);
	connect(addFilterButton, &QToolButton::clicked, this, &FilterBar::handle_newFilterButtonPressed);
	connect(filterWizard, &FilterWizard::accepted, this, &FilterBar::handle_filterWizardAccepted);

	// Create filter shortcuts
	addFilterMenu.setTitle("New filter");
	
	createFilterShortcuts.clear();
	for (const Column* const column : compTable->baseTable.getColumnList()) {
		if (column->isPrimaryKey()) continue;
		
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
void FilterBar::insertFiltersIntoUI(const QList<Filter*>& filters)
{
	resetUI();
	
	for (Filter* const filter : filters) {
		FilterBox* const newFilterBox = filter->createFilterBox(filtersScrollAreaWidget);
		
		filterBoxes.append(newFilterBox);
		filtersScrollAreaLayout->addWidget(newFilterBox);
		
		connect(newFilterBox, &FilterBox::filterChanged,	this, &FilterBar::handle_filtersChanged);
		connect(newFilterBox, &FilterBox::removeRequested,	this, &FilterBar::handle_removeFilter);
	}
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
	
	bool anyFilterEnabled = false;
	for (const FilterBox* const filterBox : filterBoxes) {
		anyFilterEnabled |= filterBox->isChecked();
	}
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(anyFilterEnabled);
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
	
	clearFiltersButton->setEnabled(compTable->filterIsActive());
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
	
	Filter* newFilter = filterWizard->getFinishedFilter();
	FilterBox* newFilterBox = newFilter->createFilterBox(filtersScrollAreaWidget);
	filterBoxes.append(newFilterBox);
	filtersScrollAreaLayout->addWidget(newFilterBox);
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
	filtersScrollArea->ensureWidgetVisible(newFilterBox);
	
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
	const NormalTable& tableToFilter = compTable->baseTable;
	const NumericFoldOp foldOp = NumericFoldOp(-1);
	
	Filter* newFilter = nullptr;
	switch (columnToUse->type) {
	case Integer:	newFilter = new IntFilter		(tableToFilter, *columnToUse, foldOp,	name);	break;
	case ID:		newFilter = new IDFilter		(tableToFilter, *columnToUse,			name);	break;
	case Enum:		newFilter = new EnumFilter		(tableToFilter, *columnToUse,			name);	break;
	case DualEnum:	newFilter = new DualEnumFilter	(tableToFilter, *columnToUse,			name);	break;
	case Bit:		newFilter = new BoolFilter		(tableToFilter, *columnToUse,			name);	break;
	case String:	newFilter = new StringFilter	(tableToFilter, *columnToUse, foldOp,	name);	break;
	case Date:		newFilter = new DateFilter		(tableToFilter, *columnToUse,			name);	break;
	case Time:		newFilter = new TimeFilter		(tableToFilter, *columnToUse,			name);	break;
	default: assert(false);
	}
	
	FilterBox* const newFilterBox = newFilter->createFilterBox(filtersScrollAreaWidget);
	filterBoxes.append(newFilterBox);
	filtersScrollAreaLayout->addWidget(newFilterBox);
	
	QApplication::processEvents();
	filtersScrollArea->setMinimumHeight(filtersScrollAreaWidget->height() + filtersScrollArea->height() - filtersScrollArea->maximumViewportSize().height());
	filtersScrollArea->ensureWidgetVisible(newFilterBox);
	
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
	
	QList<const Filter*> filters = collectEnabledFilters();
	compTable->applyFilters(filters);
	
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
 * Assembles the list of all filters currently shown in the UI.
 * 
 * @return	A list representing all filters currently shown in the UI.
 */
QList<const Filter*> FilterBar::collectAllFilters()
{
	QList<const Filter*> filters = QList<const Filter*>();
	
	for (const FilterBox* const filterBox : filterBoxes) {
		filters.append(&filterBox->getFilter());
	}
	
	return filters;
}

/**
 * Assembles the list of all filters currently active in the UI.
 *
 * Any filter whose checkbox is enabled is included in the set, and vice versa.
 *
 * @return	A list representing the active filters currently shown in the UI.
 */
QList<const Filter*> FilterBar::collectEnabledFilters()
{
	QList<const Filter*> filters = QList<const Filter*>();
	
	for (const FilterBox* const filterBox : filterBoxes) {
		if (filterBox->isChecked()) {
			filters.append(&filterBox->getFilter());
		}
	}
	
	return filters;
}



// SAVING FILTERS

/**
 * Saves the current filters to the project settings.
 * 
 * All previous filters will be cleared from the project settings.
 */
void FilterBar::saveFilters()
{
	const QList<const Filter*> filters = collectAllFilters();
	const QString encodedFilters = Filter::encodeToString(filters);
	mapper->filtersSetting.set(*this, encodedFilters);
}



// RETRIEVING FILTERS FROM PROJECT SETTINGS

/**
 * Retrieves the list of filters saved in the project settings.
 * 
 * @return	A list of filters representing the ones saved in the project settings.
 */
QList<Filter*> FilterBar::parseFiltersFromProjectSettings()
{
	const QString encodedFilters = mapper->filtersSetting.get(this);
	QList<Filter*> filters = Filter::decodeFromString(encodedFilters, mapper->baseTable.db);
	return filters;
}
