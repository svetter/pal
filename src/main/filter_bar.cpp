/*
 * Copyright 2023-2025 Simon Vetter
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
	quickFilterMenu(QMenu(this)),
	quickFilterActions(QHash<QAction*, const CompositeColumn*>()),
	filterWizard(nullptr)
{
	setupUi(this);
	
	applyFiltersButton->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
	clearFiltersButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
	filtersScrollArea->setBackgroundRole(QPalette::Base);
	
	quickFilterMenu.setTitle("New filter");
	addFilterButton->setMenu(&quickFilterMenu);
	
	
	connect(applyFiltersButton,	&QPushButton::clicked,	this,	&FilterBar::handle_applyFilters);
	connect(clearFiltersButton,	&QPushButton::clicked,	this,	&FilterBar::handle_clearFilters);
	connect(&quickFilterMenu,	&QMenu::aboutToShow,	this,	&FilterBar::updateQuickFilterMenu);
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
	filterWizard = new FilterWizard(this, *compTable, mapper->tableView);
	connect(addFilterButton, &QToolButton::clicked, this, &FilterBar::handle_newFilterButtonPressed);
	connect(filterWizard, &FilterWizard::accepted, this, &FilterBar::handle_filterWizardAccepted);
	
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
void FilterBar::insertFiltersIntoUI(const QList<Filter*>& filters, const bool filtersApplied)
{
	resetUI();
	
	for (Filter* const filter : filters) {
		FilterBox* const newFilterBox = filter->createFilterBox(filtersScrollAreaWidget);
		
		filterBoxes.append(newFilterBox);
		filtersScrollAreaLayout->addWidget(newFilterBox);
		
		connect(newFilterBox, &FilterBox::filterChanged,	this, &FilterBar::handle_filtersChanged);
		connect(newFilterBox, &FilterBox::removeRequested,	this, &FilterBar::handle_removeFilter);
	}
	
	updateScrollAreaHeight();
	
	bool anyFilterEnabled = false;
	for (const FilterBox* const filterBox : std::as_const(filterBoxes)) {
		if (filterBox->isChecked()) {
			anyFilterEnabled = true;
			break;
		}
	}
	applyFiltersButton->setEnabled(!filtersApplied && anyFilterEnabled);
	clearFiltersButton->setEnabled(filtersApplied && anyFilterEnabled);
}



// UPDATE UI

void FilterBar::updateQuickFilterMenu()
{
	quickFilterActions.clear();
	quickFilterMenu.clear();
	
	QList<QAction*> hiddenColumnActions = QList<QAction*>();
	for (int visualIndex = 0; visualIndex < compTable->getNumberOfNormalColumns(); visualIndex++) {
		const int logicalIndex = mapper->tableView.horizontalHeader()->logicalIndex(visualIndex);
		const CompositeColumn& column = compTable->getColumnAt(logicalIndex);
		const bool hidden = compTable->isColumnHidden(column);
		
		QAction* action = new QAction(column.uiName);
		if (hidden) {
			hiddenColumnActions.append(action);
		} else {
			quickFilterMenu.addAction(action);
		}
		quickFilterActions.insert(action, &column);
		connect(action, &QAction::triggered, this, &FilterBar::handle_quickFilterActionUsed);
	}
	
	if (!hiddenColumnActions.isEmpty()) {
		quickFilterMenu.addSection(tr("Hidden columns"));
		quickFilterMenu.addActions(hiddenColumnActions);
	}
}

void FilterBar::updateScrollAreaHeight()
{
	QApplication::processEvents();
	const int contentsHeight	= filtersScrollAreaWidget->height();
	const int currentHeight		= filtersScrollArea->height();
	const int viewportHeight	= filtersScrollArea->maximumViewportSize().height();
	filtersScrollArea->setMinimumHeight(contentsHeight + currentHeight - viewportHeight);
}

void FilterBar::updateIDCombos()
{
	for (const FilterBox* const filterBox : std::as_const(filterBoxes)) {
		if (filterBox->type == ID) {
			IDFilterBox* idFilterBox = (IDFilterBox*) filterBox;
			idFilterBox->setComboEntries();
		}
	}
}



void FilterBar::removeFilter(FilterBox* filterBox)
{
	filtersScrollAreaLayout->removeWidget(filterBox);
	filterBoxes.removeAll(filterBox);
	delete filterBox;
	
	updateScrollAreaHeight();
	
	handle_filtersChanged();
}

void FilterBar::compColumnAboutToBeRemoved(const CompositeColumn& column)
{
	for (int i = filterBoxes.size() - 1; i >= 0; i--) {
		const FilterBox& filterBox = *filterBoxes.at(i);
		const Filter& filter = filterBox.getFilter();
		if (&filter.columnToFilterBy == &column) {
			removeFilter(filterBoxes.at(i));
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
	for (const FilterBox* const filterBox : std::as_const(filterBoxes)) {
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
	
	updateScrollAreaHeight();
	filtersScrollArea->ensureWidgetVisible(newFilterBox);
	
	connect(newFilterBox, &FilterBox::filterChanged,	this, &FilterBar::handle_filtersChanged);
	connect(newFilterBox, &FilterBox::removeRequested,	this, &FilterBar::handle_removeFilter);
	
	handle_filtersChanged();
}

void FilterBar::handle_quickFilterActionUsed()
{
	QAction* const action = (QAction*) sender();
	assert(action);
	const QString uiName = action->text();
	const CompositeColumn* const columnToUse = quickFilterActions.value(action);
	assert(columnToUse);
	
	bool proxyIDMode;
	FilterWizard::columnEligibleForProxyIDMode(*columnToUse, &proxyIDMode);
	const DataType type = proxyIDMode ? ID : columnToUse->contentType;
	
	Filter* newFilter = nullptr;
	switch (type) {
	case Integer:	newFilter = new IntFilter		(*compTable, *columnToUse, uiName);	break;
	case ID:		newFilter = new IDFilter		(*compTable, *columnToUse, uiName);	break;
	case Enum:		newFilter = new EnumFilter		(*compTable, *columnToUse, uiName);	break;
	case DualEnum:	newFilter = new DualEnumFilter	(*compTable, *columnToUse, uiName);	break;
	case Bit:		newFilter = new BoolFilter		(*compTable, *columnToUse, uiName);	break;
	case String:	newFilter = new StringFilter	(*compTable, *columnToUse, uiName);	break;
	case Date:		newFilter = new DateFilter		(*compTable, *columnToUse, uiName);	break;
	case Time:		newFilter = new TimeFilter		(*compTable, *columnToUse, uiName);	break;
	default: assert(false);
	}
	
	FilterBox* const newFilterBox = newFilter->createFilterBox(filtersScrollAreaWidget);
	filterBoxes.append(newFilterBox);
	filtersScrollAreaLayout->addWidget(newFilterBox);
	
	updateScrollAreaHeight();
	filtersScrollArea->ensureWidgetVisible(newFilterBox);
	
	connect(newFilterBox, &FilterBox::filterChanged,	this, &FilterBar::handle_filtersChanged);
	connect(newFilterBox, &FilterBox::removeRequested,	this, &FilterBar::handle_removeFilter);
	
	handle_filtersChanged();
}

void FilterBar::handle_removeFilter()
{
	FilterBox* const filterBox = (FilterBox*) sender();
	removeFilter(filterBox);
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
	
	for (const FilterBox* const filterBox : std::as_const(filterBoxes)) {
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
	
	for (const FilterBox* const filterBox : std::as_const(filterBoxes)) {
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
	const bool filtersApplied = compTable->filterIsActive();
	const QString encodedFilters = Filter::encodeToString(filters, filtersApplied);
	mapper->filtersSetting.set(*this, encodedFilters);
}



// RETRIEVING FILTERS FROM PROJECT SETTINGS

/**
 * Retrieves the list of filters saved in the project settings.
 * 
 * @return	A list of filters representing the ones saved in the project settings.
 */
QList<Filter*> FilterBar::parseFiltersFromProjectSettings(const ItemTypesHandler& typesHandler, bool* const filtersAppliedResult)
{
	const QString encodedFilters = mapper->filtersSetting.get(this);
	QList<Filter*> filters = Filter::decodeFromString(encodedFilters, typesHandler, filtersAppliedResult);
	return filters;
}



void FilterBar::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	updateScrollAreaHeight();
}
