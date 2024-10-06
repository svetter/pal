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
 * @file main_window.cpp
 * 
 * This file defines the MainWindow class.
 */

#include "main_window.h"

#include "src/main/about_window.h"
#include "src/data/item_types.h"
#include "src/settings/project_settings_window.h"
#include "src/settings/settings_window.h"
#include "src/tools/peak_links_dialog.h"
#include "src/tools/relocate_photos_dialog.h"
#include "src/tools/export_dialog.h"
#include "src/viewer/ascent_viewer.h"
#include "ui_main_window.h"



/**
 * Creates a new MainWindow.
 * 
 * Initializes the Database instance and the UI and creates an ItemTypesHandler. Then opens the
 * most recently opened database file, if present.
 */
MainWindow::MainWindow() :
	QMainWindow(nullptr),
	Ui_MainWindow(),
	projectOpen(false),
	db(Database()),
	typesHandler(nullptr),
	openRecentActions(QList<QAction*>()),
	statusBarTableSizeLabel(new QLabel(statusbar)),
	statusBarFiltersLabel(new QLabel(statusbar)),
	generalStatsEngine(GeneralStatsEngine(db, &statisticsTabLayout))
{
	setupUi(this);
	createTypesHandler();
	setWindowIcon(QIcon(":/icons/ico/logo_peak_multisize_square.ico"));
	setupMenuIcons();
	statusbar->addPermanentWidget(statusBarTableSizeLabel);
	statusbar->addPermanentWidget(statusBarFiltersLabel);
	setUIEnabled(false);
	
	setWindowTitleFilename();
	
	if (Settings::rememberWindowPositions.get()) {
		setWindowState(Settings::mainWindow_maximized.get() ? Qt::WindowMaximized : Qt::WindowNoState);
		const QRect savedGeometry = Settings::mainWindow_geometry.get();
		if (!savedGeometry.isEmpty()) {
			setGeometry(savedGeometry);
		}
	}
	
	pinStatsRangesAction->setChecked(Settings::itemStats_pinRanges.get());
	
	for (ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->filterBar.supplyPointers(this, &db, mapper);
	}
	
	
	connectUI();
	setupTableTabs();
	generalStatsEngine.setupStatsTab();
	updateItemCountDisplays(true);
	updateRecentFilesMenu();
	
	// Open database
	QString lastOpen = Settings::lastOpenDatabaseFile.get();
	if (!lastOpen.isEmpty() && QFile(lastOpen).exists()) {
		attemptToOpenFile(lastOpen);
	}
}

/**
 * Destroys the MainWindow.
 */
MainWindow::~MainWindow()
{
	delete typesHandler;
}



// INITIAL SETUP

/**
 * Creates the ItemTypesHandler singleton.
 */
void MainWindow::createTypesHandler()
{
	typesHandler = new ItemTypesHandler(db,
		{
			{ItemTypeAscent,	TypeMapperPointers{ascentsTab,		newAscentAction,	newAscentButton		} },
			{ItemTypePeak,		TypeMapperPointers{peaksTab,		newPeakAction,		newPeakButton		} },
			{ItemTypeTrip,		TypeMapperPointers{tripsTab,		newTripAction,		newTripButton		} },
			{ItemTypeHiker,		TypeMapperPointers{hikersTab,		newHikerAction,		newHikerButton		} },
			{ItemTypeRegion,	TypeMapperPointers{regionsTab,		newRegionAction,	newRegionButton		} },
			{ItemTypeRange,		TypeMapperPointers{rangesTab,		newRangeAction,		newRangeButton		} },
			{ItemTypeCountry,	TypeMapperPointers{countriesTab,	newCountryAction,	newCountryButton	} }
		}
	);
}

/**
 * Adds standard icons to some menu items.
 */
void MainWindow::setupMenuIcons()
{
	// File menu
	newDatabaseAction			->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
	openDatabaseAction			->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	openRecentMenu				->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	saveDatabaseAsAction		->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	closeDatabaseAction			->setIcon(style()->standardIcon(QStyle::SP_TabCloseButton));
	projectSettingsAction		->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	settingsAction				->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	
	// View menu
	// showFiltersAction is checkable
	// showItemStatsPanelAction is checkable
	showAllStatsPanelsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	hideAllStatsPanelsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	// pinStatsRangesAction is checkable
	autoResizeColumnsAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	resetColumnOrderAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	restoreHiddenColumnsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	addCustomColumnAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	clearTableSelectionAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	
	// New menu: using own icons
	
	// Tools menu
	findPeakLinksAction			->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	relocatePhotosAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	exportDataAction			->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	
	// Help menu: already has icons
}

/**
 * Connects all UI elements to their respective handlers.
 */
void MainWindow::connectUI()
{
	// === MENU ITEMS AND BUTTONS ===
	
	// Menu "File"
	connect(newDatabaseAction,				&QAction::triggered,			this,	&MainWindow::handle_newDatabase);
	connect(openDatabaseAction,				&QAction::triggered,			this,	&MainWindow::handle_openDatabase);
	connect(clearRecentDatabaseListAction,	&QAction::triggered,			this,	&MainWindow::handle_clearRecentDatabasesList);
	connect(saveDatabaseAsAction,			&QAction::triggered,			this,	&MainWindow::handle_saveDatabaseAs);
	connect(closeDatabaseAction,			&QAction::triggered,			this,	&MainWindow::handle_closeDatabase);
	connect(projectSettingsAction,			&QAction::triggered,			this,	&MainWindow::handle_openProjectSettings);
	connect(settingsAction,					&QAction::triggered,			this,	&MainWindow::handle_openSettings);
	
	// Menu "View"
	connect(showFiltersAction,				&QAction::toggled,				this,	&MainWindow::handle_showFiltersChanged);
	connect(showItemStatsPanelAction,		&QAction::toggled,				this,	&MainWindow::handle_showStatsPanelChanged);
	connect(showAllStatsPanelsAction,		&QAction::triggered,			this,	&MainWindow::handle_showAllStatsPanels);
	connect(hideAllStatsPanelsAction,		&QAction::triggered,			this,	&MainWindow::handle_hideAllStatsPanels);
	connect(pinStatsRangesAction,			&QAction::triggered,			this,	&MainWindow::handle_chartRangesPinnedChanged);
	connect(autoResizeColumnsAction,		&QAction::triggered,			this,	&MainWindow::handle_autoResizeColumns);
	connect(resetColumnOrderAction,			&QAction::triggered,			this,	&MainWindow::handle_resetColumnOrder);
	connect(restoreHiddenColumnsAction,		&QAction::triggered,			this,	&MainWindow::handle_restoreHiddenColumns);
	connect(addCustomColumnAction,			&QAction::triggered,			this,	&MainWindow::handle_addCustomColumn);
	connect(clearTableSelectionAction,		&QAction::triggered,			this,	&MainWindow::handle_clearTableSelection);
	
	// Menu "New"
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		auto newFunction = [this, mapper] () {
			newItem(*mapper);
		};
		
		connect(&mapper->newItemAction,		&QAction::triggered,			this,	newFunction);
		connect(&mapper->newItemButton,		&QPushButton::clicked,			this,	newFunction);
	}
	
	// Menu "Tools"
	connect(findPeakLinksAction,			&QAction::triggered,			this,	&MainWindow::handle_findPeakLinks);
	connect(relocatePhotosAction,			&QAction::triggered,			this,	&MainWindow::handle_relocatePhotos);
	connect(exportDataAction,				&QAction::triggered,			this,	&MainWindow::handle_exportData);
	
	// Menu "Help"
	connect(aboutAction,					&QAction::triggered,			this,	&MainWindow::handle_about);
	connect(aboutQtAction,					&QAction::triggered,			this,	&QApplication::aboutQt);
	
	
	// === TABS AND TABLES ===
	
	// Tabs
	connect(mainAreaTabs,					&QTabWidget::currentChanged,	this,	&MainWindow::handle_tabChanged);
	// Double clicks on table
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		auto handlerFunction = &MainWindow::editSelectedItems;
		if (mapper->type == ItemTypeAscent) {
			handlerFunction = &MainWindow::viewSelectedItem;
		}
		connect(&mapper->tableView,			&QTableView::doubleClicked,		this,	handlerFunction);
		connect(&mapper->compTable,			&CompositeTable::wasResorted,	this,	&MainWindow::scrollToTopAfterSorting);
	}
}

/**
 * Supplies basic information to each tab content widget and has them set up.
 */
void MainWindow::setupTableTabs()
{
	for (ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		const bool isViewable	= mapper->type == ItemTypeAscent;
		const bool isDuplicable	= mapper->type == ItemTypeAscent || mapper->type == ItemTypePeak;
		
		mapper->tab.init(this, typesHandler, mapper, db, isViewable, isDuplicable);
	}
}



// PROJECT SETUP

/**
 * Attempts to open the given file and only changes UI if database initialization is successful.
 * 
 * @param filepath	The file to attempt to open.
 */
void MainWindow::attemptToOpenFile(const QString& filepath)
{
	assert(!projectOpen);
	
	setVisible(true);
	updateTopBarButtonVisibilities();
	
	bool dbOpened = db.openExisting(*this, filepath);
	
	if (dbOpened) {
		setWindowTitleFilename(filepath);
		updateFilterCombos();
		
		// Restore project-specific implicit settings:
		// Open tab
		int tabIndex = 0;
		if (Settings::rememberTab.get()) {
			tabIndex = db.projectSettings.mainWindow_currentTabIndex.get(this);
		}
		mainAreaTabs->setCurrentIndex(tabIndex);
		
		ItemTypeMapper* activeMapper = getActiveMapperOrNull();
		if (activeMapper) {
			activeMapper->openingTab();
			showItemStatsPanelAction->setChecked(activeMapper->itemStatsPanelCurrentlySetVisible());
			activeMapper->statsEngine.setCurrentlyVisible(true, true);
			showFiltersAction->setChecked(activeMapper->filterBarCurrentlySetVisible());
		}
		generalStatsEngine.setCurrentlyVisible(!activeMapper);
		
		for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
			// Custom columns
			const QString encodedCustomColumns = mapper->customColumnsSetting.get();
			const QList<CompositeColumn*> customColumns = CompositeColumn::decodeFromString(encodedCustomColumns, db, *typesHandler);
			mapper->compTable.setCustomColumns(customColumns);
			// Filter bar
			mapper->filterBar.setVisible(mapper->showFilterBarSetting.get(this));
			// Column widths
			if (Settings::rememberColumnWidths.get()) {
				mapper->tab.restoreColumnWidths();
			}
			// Column order
			if (Settings::rememberColumnOrder.get()) {
				mapper->tab.restoreColumnOrder();
			}
			// Hidden columns
			if (Settings::rememberHiddenColumns.get()) {
				mapper->tab.restoreColumnHiddenStatus();
			}
			// Sortings
			mapper->tab.setSorting();
		}
		
		// Build buffers and update size info
		initCompositeBuffers();
		projectOpen = true;
		
		updateItemCountDisplays();
		getActiveMapper().tab.refreshStats();
		
		setUIEnabled(true);
		addToRecentFilesList(filepath);
	}
}

/**
 * Prepares the composite tables and fills either all of them or only the one currently being
 * shown, with a status bar progress dialog.
 * 
 * If the user setting is to *not* prepare all tables on startup, all tables are still being
 * initialized (so as not to produce null pointer exceptions), but not filled with data.
 */
void MainWindow::initCompositeBuffers()
{
	QProgressDialog progress(this);
	progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowCloseButtonHint);
	progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Opening database"));
	progress.setMinimumWidth(250);
	progress.setCancelButton(nullptr);
	progress.setMinimumDuration(100);
	progress.setLabel(new QLabel(tr("Preparing tables..."), &progress));
	
	bool prepareAll = !Settings::onlyPrepareActiveTableOnStartup.get();
	const ItemTypeMapper* const activeMapper = getActiveMapperOrNull();
	const QTableView* const currentTableView = activeMapper ? &activeMapper->tableView : nullptr;
	
	int numCells = 0;
	if (prepareAll) {
		for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
			numCells += mapper->compTable.getNumberOfCellsToInit();
		}
	} else {
		if (activeMapper) {
			numCells += activeMapper->compTable.getNumberOfCellsToInit();
		}
	}
	progress.setMinimum(0);
	progress.setMaximum(numCells);
	progress.setValue(0);
	
	for (ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		progress.setLabelText(tr("Preparing table %1...").arg(mapper->baseTable.uiName));
		
		const bool isOpen = &mapper->tableView == currentTableView;
		
		// Load filters
		QList<Filter*> filters = QList<Filter*>();
		if (Settings::rememberFilters.get()) {
			filters = mapper->filterBar.parseFiltersFromProjectSettings(*typesHandler);
			mapper->filterBar.insertFiltersIntoUI(filters);
			if (!mapper->filterBarCurrentlySetVisible()) {
				// Hidden filters are not allowed to be applied
				filters.clear();
			}
		}
		QList<const Filter*> filtersToApply = QList<const Filter*>();
		for (const Filter* const filter : filters) {
			if (filter->isEnabled()) filtersToApply.append(filter);
		}
		mapper->compTable.setInitialFilters(filtersToApply);
		
		// Check whether table needs to be fully prepared
		const bool prepareThisTable = prepareAll || isOpen;
		
		// Check whether columns should be auto-resized after preparation
		const QSet<QString> columnNameSet = mapper->compTable.getNormalColumnNameSet();
		const bool autoResizeColumns = !Settings::rememberColumnWidths.get() || mapper->columnWidthsSetting.nonePresent(columnNameSet);
		
		// Collect buffer initialization parameters
		QProgressDialog* const updateProgress = prepareThisTable ? &progress : nullptr;
		const bool deferCompute = !prepareThisTable;
		QTableView* const tableToAutoResizeAfterCompute = autoResizeColumns ? &mapper->tableView : nullptr;
		
		mapper->compTable.initBuffer(updateProgress, deferCompute, tableToAutoResizeAfterCompute);
		if (isOpen) mapper->openingTab();
	}
}



// UI UPDATES

/**
 * Sets the enabled state of all UI elements except the project-independent parts of the menu bar.
 * 
 * @param enabled	The new enabled state.
 */
void MainWindow::setUIEnabled(bool enabled)
{
	const int currentTabIndex = mainAreaTabs->currentIndex();
	const bool statsTabOpen = currentTabIndex == mainAreaTabs->indexOf(statisticsTab);
	
	saveDatabaseAsAction	->setEnabled(enabled);
	closeDatabaseAction		->setEnabled(enabled);
	projectSettingsAction	->setEnabled(enabled);
	viewMenu				->setEnabled(enabled && !statsTabOpen);
	newMenu					->setEnabled(enabled);
	toolsMenu				->setEnabled(enabled);
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->newItemButton.setEnabled(enabled);
	}
	mainAreaTabs						->setEnabled(enabled);
	ascentCounterSegmentNumber			->setEnabled(enabled);
	ascentCounterLabel					->setEnabled(enabled);
	ascentCounterFilteredSegmentNumber	->setEnabled(enabled);
	ascentCounterFilteredLabel			->setEnabled(enabled);
}

/**
 * Updates the list of recently opened database files in the "File" menu.
 */
void MainWindow::updateRecentFilesMenu()
{
	openRecentMenu->clear();
	
	QStringList recentFiles = Settings::recentDatabaseFiles.get();
	for (QString& filepath : recentFiles) {
		if (!QFile(filepath).exists()) recentFiles.removeAll(filepath);
	}
	Settings::recentDatabaseFiles.set(recentFiles);
	
	if (recentFiles.isEmpty()) {
		openRecentMenu->setEnabled(false);
		return;
	}
	
	for (QString& filepath : recentFiles) {
		auto handler = [=](){ handle_openRecentDatabase(filepath); };
		QAction* newAction = openRecentMenu->addAction(filepath, this, handler);
		newAction->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
		openRecentActions.append(newAction);
	}
	
	openRecentMenu->addSeparator();
	openRecentMenu->addAction(clearRecentDatabaseListAction);
	openRecentMenu->setEnabled(true);
}

/**
 * Updates or clears the status bar indicator for the number of entries in the currently selected
 * table, as well as the LED-styled ascent counters at the top.
 * 
 * For the ascent table, also updates the status bar indicator for the number of applied filters.
 * 
 * @param reset	If true, clear the status bar indicator and LED counters completely.
 */
void MainWindow::updateItemCountDisplays(bool reset)
{
	if (reset) {
		statusBarTableSizeLabel->setText("");
		statusBarFiltersLabel->setText("");
		ascentCounterSegmentNumber->setProperty("value", QVariant());
		ascentCounterFilteredSegmentNumber->setProperty("value", QVariant());
		setFilteredAscentsCounterVisible(false);
		return;
	}
	
	const ItemTypeMapper* const activeMapper = getActiveMapperOrNull();
	
	QString countText = QString();
	if (activeMapper) {
		const int total = activeMapper->baseTable.getNumberOfRows();
		if (total == 0) {
			countText = tr("Table is empty");
		}
		else if (activeMapper->type == ItemTypeAscent) {
			int displayed = activeMapper->compTable.rowCount();
			int filtered = total - displayed;
			countText = (total == 1 ? tr("%2 of %1 entry shown (%3 filtered out)") : tr("%2 of %1 entries shown (%3 filtered out)")).arg(total).arg(displayed).arg(filtered);
		} else {
			countText = (total == 1 ? tr("%1 entry") : tr("%1 entries")).arg(total);
		}
	}
	statusBarTableSizeLabel->setText(countText);
	
	QString filterText = QString();
	bool filteredAscentsCounterVisible = false;
	if (activeMapper && activeMapper->type == ItemTypeAscent) {
		int numFiltersApplied = activeMapper->compTable.getCurrentFilters().size();
		if (numFiltersApplied > 0) {
			filterText = (numFiltersApplied == 1 ? tr("%1 filter applied") : tr("%1 filters applied")).arg(numFiltersApplied);
			filteredAscentsCounterVisible = true;
		} else {
			filterText = tr("No filters applied");
		}
	}
	statusBarFiltersLabel->setText(filterText);
	
	ascentCounterSegmentNumber->setProperty("value", QVariant(db.ascentsTable.getNumberOfRows()));
	
	// Set number of filtered rows
	const int numAscentsShown = typesHandler->get(ItemTypeAscent).compTable.rowCount();
	ascentCounterFilteredSegmentNumber->setProperty("value", QVariant(numAscentsShown));
	setFilteredAscentsCounterVisible(filteredAscentsCounterVisible);
}



/**
 * To be called externally after filters applied to the current composite table have changed.
 * 
 * Updates the table size info and item statistics, if shown.
 */
void MainWindow::currentFiltersChanged()
{
	updateItemCountDisplays();
	handle_clearTableSelection();	// Have to clear table selection for chart range pinning to work correctly
	getActiveMapper().tab.refreshStats();
}



// EXECUTE USER COMMANDS

/**
 * Opens the item at the primary selected row in the currently active table.
 */
void MainWindow::viewSelectedItem()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const BufferRowIndex primaryBufferRow = activeMapper.tab.getSelectedRows().second;
	if (primaryBufferRow.isInvalid()) return;
	
	switch (activeMapper.type) {
	case ItemTypeAscent: {
		const ViewRowIndex markedViewRow = activeMapper.compTable.findViewRowIndexForBufferRow(primaryBufferRow);
		AscentViewer* viewer = new AscentViewer(this, db, typesHandler, markedViewRow);
		connect(viewer, &AscentViewer::finished, [=]() { delete viewer; });
		viewer->open();
		break;
	}
	default: assert(false);
	}
}

/**
 * Opens a dialog for creating a new item of the type specified by the given ItemTypeMapper.
 * 
 * If a new item was created, performs the necessary updates to the UI.
 * 
 * @param mapper	The ItemTypeMapper for the type of item to create.
 */
void MainWindow::newItem(const ItemTypeMapper& mapper)
{
	mapper.openNewItemDialogAndStoreMethod(*this, *this, db, [this, &mapper](BufferRowIndex newBufferRowIndex) {
		if (newBufferRowIndex.isInvalid()) return;
		
		setStatusLine(mapper.baseTable.getCreationConfirmMessage());
		performUpdatesAfterUserAction(mapper, true, newBufferRowIndex);
	});
}

/**
 * Opens a dialog for creating a new item for the currently active table as a duplicate of the item
 * at the primary selected row.
 * 
 * If a new item was created, performs the necessary updates to the UI.
 */
void MainWindow::duplicateAndEditSelectedItem()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const BufferRowIndex primaryBufferRow = activeMapper.tab.getSelectedRows().second;
	if (primaryBufferRow.isInvalid()) return;
	
	activeMapper.openDuplicateItemDialogAndStoreMethod(*this, *this, db, primaryBufferRow, [this, &activeMapper](BufferRowIndex newBufferRowIndex) {
		if (newBufferRowIndex.isInvalid()) return;
		
		setStatusLine(activeMapper.baseTable.getCreationConfirmMessage());
		performUpdatesAfterUserAction(activeMapper, true, newBufferRowIndex);
	});
}

/**
 * Opens a dialog for editing the items at the selected rows in the currently active table.
 * 
 * If the items were edited, performs the necessary updates to the UI.
 */
void MainWindow::editSelectedItems()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const QPair<QSet<BufferRowIndex>, BufferRowIndex> selectedAndMarkedBufferRows = activeMapper.tab.getSelectedRows();
	const QSet<BufferRowIndex>& selectedBufferRows = selectedAndMarkedBufferRows.first;
	const BufferRowIndex markedBufferRow = selectedAndMarkedBufferRows.second;
	if (selectedBufferRows.isEmpty()) return;
	
	const int numSelectedRows = selectedBufferRows.size();
	auto callWhenDone = [=, &activeMapper](bool changesMade) {
		if (!changesMade) return;
		setStatusLine(activeMapper.baseTable.getEditConfirmMessage(numSelectedRows));
		performUpdatesAfterUserAction(activeMapper, false);
	};
	
	if (selectedBufferRows.size() == 1) {
		activeMapper.openEditItemDialogAndStoreMethod(*this, *this, db, markedBufferRow, callWhenDone);
	} else {
		activeMapper.openMultiEditItemsDialogAndStoreMethod(*this, *this, db, selectedBufferRows, markedBufferRow, callWhenDone);
	}
}

/**
 * If a single item is selected in the active table, opens a dialog for editing the item of the type
 * specified via caller data that the selected item references directly (forward only).
 */
void MainWindow::editSelectedItemReferenced()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const QSet<BufferRowIndex>& selectedBufferRows = activeMapper.tab.getSelectedRows().first;
	if (selectedBufferRows.size() != 1) return;
	const BufferRowIndex selectedBufferRow = BufferRowIndex(selectedBufferRows.constBegin()->get());
	
	const QAction* const action = (QAction*) sender();
	assert(action);
	const PALItemType otherType = PALItemType(action->data().toInt());
	const ItemTypeMapper& otherMapper = typesHandler->get(otherType);
	
	const Breadcrumbs crumbs = db.getBreadcrumbsFor(activeMapper.baseTable, otherMapper.baseTable);
	const BufferRowIndex targetBufferRow = crumbs.evaluateAsForwardChain(selectedBufferRow);
	
	auto callWhenDone = [=, &activeMapper](bool changesMade) {
		if (!changesMade) return;
		setStatusLine(activeMapper.baseTable.getEditConfirmMessage(1));
		performUpdatesAfterUserAction(activeMapper, false);
	};
	
	otherMapper.openEditItemDialogAndStoreMethod(*this, *this, db, targetBufferRow, callWhenDone);
}

/**
 * Opens a dialog for deleting the selected items in the currently active table.
 * 
 * If the item was deleted, performs the necessary updates to the UI.
 */
void MainWindow::deleteSelectedItems()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const QSet<BufferRowIndex> selectedBufferRows = activeMapper.tab.getSelectedRows().first;
	if (selectedBufferRows.isEmpty()) return;
	
	QSet<ViewRowIndex> selectedViewRowIndices = QSet<ViewRowIndex>();
	for (const QModelIndex& index : getActiveMapper().tableView.selectionModel()->selectedRows()) {
		selectedViewRowIndices += ViewRowIndex(index.row());
	}
	if (selectedViewRowIndices.isEmpty()) return;
	
	const bool deleted = activeMapper.openDeleteItemsDialogAndExecuteMethod(*this, *this, db, selectedBufferRows);
	if (!deleted) return;
	
	setStatusLine(activeMapper.baseTable.getDeleteConfirmMessage(selectedBufferRows.size()));
	performUpdatesAfterUserAction(activeMapper, true);
}


/**
 * Sets the visibility for the filtered ascents counter in the top bar.
 * 
 * @param visible	Whether the filtered ascents counter should be visible.
 */
void MainWindow::setFilteredAscentsCounterVisible(bool visible)
{
	setUpdatesEnabled(false);
	
	ascentCounterFilteredSegmentNumber->setVisible(visible);
	ascentCounterFilteredLabel->setVisible(visible);
	
	updateTopBarButtonVisibilities();
	
	setUpdatesEnabled(true);
}

/**
 * Updates the visibility of the new item buttons in the top bar according to how much space there
 * is to the left of the ascent counter labels.
 */
void MainWindow::updateTopBarButtonVisibilities()
{
	if (!isVisible()) return;
	
	const QLabel* const leftmostCounterLabel = ascentCounterFilteredLabel->isVisible() ? ascentCounterFilteredLabel : ascentCounterLabel;
	
	setUpdatesEnabled(false);
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		QPushButton& button = mapper->newItemButton;
		const bool buttonVisible = button.x() + button.width() < leftmostCounterLabel->x() - 40;
		button.setVisible(buttonVisible);
	}
	setUpdatesEnabled(true);
}

/**
 * Updates selection, table size info and filters after an item was added, edited or removed.
 * 
 * @param mapper					The ItemTypeMapper for the table that was changed.
 * @param numberOfEntriesChanged	Whether the number of entries in the table changed.
 * @param bufferRowToSelectIndex	The buffer row index of the item to select after the update.
 */
void MainWindow::performUpdatesAfterUserAction(const ItemTypeMapper& mapper, bool numberOfEntriesChanged, BufferRowIndex bufferRowToSelectIndex)
{
	// Update selection in table
	if (bufferRowToSelectIndex.isValid()) {
		ViewRowIndex viewRowToSelectIndex = mapper.compTable.findViewRowIndexForBufferRow(bufferRowToSelectIndex);
		updateSelectionAfterUserAction(mapper, viewRowToSelectIndex);
	} else {
		if (numberOfEntriesChanged) handle_clearTableSelection();
		mapper.tab.refreshStats();
	}
	// Update table size info
	if (numberOfEntriesChanged) updateItemCountDisplays();
	// Update filters
	updateFilterCombos();
}

/**
 * Scrolls the active table to the top.
 * 
 * To be called after the table was sorted.
 */
void MainWindow::scrollToTopAfterSorting()
{
	ItemTypeMapper* activeMapper = getActiveMapperOrNull();
	if (!activeMapper) return;
	
	QTableView& tableView = activeMapper->tableView;
	const int horizontalScroll = tableView.horizontalScrollBar()->value();
	tableView.scrollToTop();
	tableView.horizontalScrollBar()->setValue(horizontalScroll);
}

/**
 * Updates elements of the filter bars which depend on contents of the database.
 * 
 * @param onlyForMapper	The ItemTypeMapper for the table that was changed. Set to nullptr to update all filter bars.
 */
void MainWindow::updateFilterCombos(const ItemTypeMapper* onlyForMapper)
{
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		if (!onlyForMapper || mapper == onlyForMapper) {
			mapper->filterBar.updateIDCombos();
		}
	}
}

/**
 * Updates the selection in the table view specified by the given ItemTypeMapper to the item at the
 * given view row index.
 * 
 * @param mapper		The ItemTypeMapper containing the table view whose selection should be updated.
 * @param viewRowIndex	The view row index of the item to select.
 */
void MainWindow::updateSelectionAfterUserAction(const ItemTypeMapper& mapper, ViewRowIndex viewRowIndex)
{
	QModelIndex modelIndex = mapper.compTable.index(viewRowIndex.get(), 0);
	mapper.tableView.setCurrentIndex(modelIndex);
	mapper.tableView.scrollTo(modelIndex);
}



// UI EVENT HANDLERS

/**
 * Event handler for changes in which tab is selected in the main tab widget.
 * 
 * Prepares the newly active table if necessary (with a progress dialog) and updates the table size
 * info and the enabled state of the table context menu actions.
 */
void MainWindow::handle_tabChanged()
{
	if (!projectOpen) return;
	
	ItemTypeMapper* const activeMapper = getActiveMapperOrNull();
	
	updateItemCountDisplays();
	
	QProgressDialog progress(this);
	progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowCloseButtonHint);
	progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Updating table"));
	progress.setLabel(new QLabel(tr("Updating table..."), &progress));
	progress.setMinimumWidth(250);
	progress.setCancelButton(nullptr);
	progress.setMinimumDuration(100);
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		if (mapper == activeMapper) continue;
		
		mapper->compTable.setUpdateImmediately(false);
		mapper->statsEngine.setCurrentlyVisible(false);
	}
	
	if (activeMapper) {
		// Make sure active table is up to date
		progress.setMaximum(activeMapper->compTable.getNumberOfCellsToUpdate());
		activeMapper->compTable.setUpdateImmediately(true, &progress);
		
		const bool firstOpen = !activeMapper->tabHasBeenOpened(false);
		activeMapper->openingTab();
		
		// Filter bar visibility
		const bool filterBarShown = activeMapper->filterBarCurrentlySetVisible();
		showFiltersAction->setChecked(filterBarShown);
		
		// Item stats panel visibility
		const bool statsShown = activeMapper->itemStatsPanelCurrentlySetVisible();
		showItemStatsPanelAction->setChecked(statsShown);
		activeMapper->statsEngine.setCurrentlyVisible(statsShown, true);
		if (firstOpen) activeMapper->tab.refreshStats();
	}
	generalStatsEngine.setCurrentlyVisible(!activeMapper);
	
	setUIEnabled(true);
}



// FILE MENU ACTION HANDLERS

/**
 * Event handler for the "new database" action in the file menu.
 * 
 * Prompts the user for a filepath, closes the currently open database (if any), creates a new
 * database at the given filepath and opens it.
 * The filepath is added to the top of the list of recently opened databases.
 * If the corresponding user setting is enabled, the project settings dialog is opened.
 */
void MainWindow::handle_newDatabase()
{
	QString caption = tr("Save new database as");
	QString preSelectedDir = Settings::lastOpenDatabaseFile.get();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString filepath = QFileDialog::getSaveFileName(this, caption, preSelectedDir);
	if (filepath.isEmpty()) return;
	
	if (!filepath.endsWith(".db")) filepath.append(".db");
	
	if (projectOpen) handle_closeDatabase();
	
	setWindowTitleFilename(filepath);
	db.createNew(*this, filepath);
	
	// Build buffers and update size info
	initCompositeBuffers();
	projectOpen = true;
	
	ItemTypeMapper& activeMapper = getActiveMapper();
	
	updateFilterCombos();
	updateItemCountDisplays();
	activeMapper.tab.refreshStats();
	setUIEnabled(true);
	
	addToRecentFilesList(filepath);
	
	activeMapper.openingTab();
	activeMapper.statsEngine.setCurrentlyVisible(true);
	
	ProjectSettingsWindow* dialog = new ProjectSettingsWindow(*this, *this, db, true);
	connect(dialog, &ProjectSettingsWindow::finished, [=]() { delete dialog; });
	dialog->open();
}

/**
 * Event handler for the "open database" action in the file menu.
 * 
 * Prompts the user for a filepath, closes the currently open database (if any) and opens the
 * database at the given filepath.
 * The filepath is added to the top of the list of recently opened databases.
 */
void MainWindow::handle_openDatabase()
{
	QString caption = tr("Open database");
	QString preSelectedDir = Settings::lastOpenDatabaseFile.get();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString filter = tr("Database files") + " (*.db);;" + tr("All files") + " (*.*)";
	QString filepath = QFileDialog::getOpenFileName(this, caption, preSelectedDir, filter);
	if (filepath.isEmpty() || !QFile(filepath).exists()) return;
	
	if (projectOpen) handle_closeDatabase();
	
	attemptToOpenFile(filepath);
}

/**
 * Event handler for the "open recent database" actions in the file menu.
 * 
 * Closes the currently open database (if any) and opens the database at the given filepath.
 * The filepath is added to the top of the list of recently opened databases.
 * 
 * @param filepath	The filepath of the database to open.
 */
void MainWindow::handle_openRecentDatabase(QString filepath)
{
	if (!QFile(filepath).exists()) {
		qDebug() << "Database file" << filepath << "was selected to be opened from recent files, but doesn't exist";
		return;
	}
	
	if (projectOpen) handle_closeDatabase();
	
	attemptToOpenFile(filepath);
}

/**
 * Event handler for the "clear recent databases list" action in the file menu.
 * 
 * Clears the list of recently opened databases in settings as well as the menu bar, leaving only
 * the currently open database.
 */
void MainWindow::handle_clearRecentDatabasesList()
{
	Settings::recentDatabaseFiles.set({ Settings::lastOpenDatabaseFile.get() });
	updateRecentFilesMenu();
}

/**
 * Event handler for the "save database as" action in the file menu.
 * 
 * Prompts the user for a filepath, performs necessary checks and saves the currently open database
 * to the given filepath.
 * The window title is updated and the filepath is added to the top of the list of recently opened
 * databases.
 */
void MainWindow::handle_saveDatabaseAs()
{
	assert(projectOpen);
	
	QString caption = tr("Save database as");
	QString preSelectedDir = Settings::lastOpenDatabaseFile.get();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString filepath = QFileDialog::getSaveFileName(this, caption, preSelectedDir);
	if (filepath.isEmpty()) return;
	
	if (!filepath.endsWith(".db")) {
		filepath.append(".db");
	}
	if (filepath.compare(db.getCurrentFilepath(), Qt::CaseInsensitive) == 0) {
		QString title = tr("Save database as");
		QString message = tr("You cannot Save As to the same file.") + "\n\n"
				+ tr("Hint:") + "\n" + tr("PAL auto-saves every change you make immediately, there is no need to save manually.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
		return;
	}
	if (QFile(filepath).exists()) {
		qDebug() << "Existing file needs to be removed";
		QFile(filepath).remove();
	}
	
	bool success = db.saveAs(*this, filepath);
	if (!success) {
		QString title = tr("Save database as");
		QString message = tr("Writing database file failed:")
				+ "\n" + filepath + "\n"
				+ tr("Reverting to previously opened file:")
				+ "\n" + db.getCurrentFilepath();
		QMessageBox::warning(this, title, message);
		return;
	}
	
	setWindowTitleFilename(filepath);
	addToRecentFilesList(filepath);
}

/**
 * Event handler for the "close database" action in the file menu.
 * 
 * Resets window title, database and tables and disables all UI elements except for the project-
 * independent parts of the menu bar.
 */
void MainWindow::handle_closeDatabase()
{
	assert(projectOpen);
	saveProjectImplicitSettings();
	setWindowTitleFilename();
	setUIEnabled(false);
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->filterBar.resetUI();
	}
	
	db.reset();
	projectOpen = false;
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->compTable.reset();
		mapper->statsEngine.resetStatsPanel();
	}
	generalStatsEngine.resetStatsTab();
	updateItemCountDisplays(true);
	mainAreaTabs->setCurrentIndex(0);
	typesHandler->resetTabOpenedFlags();
}


/**
 * Event handler for the "project settings" action in the file menu.
 * 
 * Opens the project settings dialog.
 */
void MainWindow::handle_openProjectSettings()
{
	ProjectSettingsWindow* dialog = new ProjectSettingsWindow(*this, *this, db);
	connect(dialog, &ProjectSettingsWindow::finished, [=]() { delete dialog; });
	dialog->open();
}

/**
 * Event handler for the "settings" action in the file menu.
 * 
 * Opens the settings dialog.
 */
void MainWindow::handle_openSettings()
{
	SettingsWindow* dialog = new SettingsWindow(*this);
	connect(dialog, &SettingsWindow::finished, [=]() { delete dialog; });
	dialog->open();
}



// VIEW MENU ACTION HANDLERS

/**
 * Event handler for the "show filters" action in the view menu.
 * 
 * Shows or hides the ascent filter bar.
 */
void MainWindow::handle_showFiltersChanged()
{
	if (!projectOpen) return;
	const ItemTypeMapper& activeMapper = getActiveMapper();
	
	if (activeMapper.filterBarCurrentlySetVisible()) {
		// No hidden filters can be active
		activeMapper.filterBar.handle_clearFilters();
	}
	
	const bool showFilters = showFiltersAction->isChecked();
	activeMapper.filterBar.setVisible(showFilters);
}

/**
 * Event handler for the "show statistics panel" action in the view menu.
 * 
 * Shows or hides the active item-related statistics panel.
 */
void MainWindow::handle_showStatsPanelChanged()
{
	if (!projectOpen) return;
	ItemTypeMapper& activeMapper = getActiveMapper();
	
	if (activeMapper.itemStatsPanelCurrentlySetVisible()) {
		// Save splitter sizes before closing
		QSplitter* const splitter = activeMapper.tab.findChild<QSplitter*>();
		saveSplitterSizes(*splitter, activeMapper.statsPanelSplitterSizesSetting);
	}
	
	const bool showStatsPanel = showItemStatsPanelAction->isChecked();
	activeMapper.statsScrollArea.setVisible(showStatsPanel);
	activeMapper.statsEngine.setCurrentlyVisible(showStatsPanel);
}

/**
 * Event handler for the "show all statistics panels" action in the view menu.
 * 
 * Sets all item-related statistics panels to visible, sets the corresponding action to checked and
 * triggers an update for the active statistics panel.
 */
void MainWindow::handle_showAllStatsPanels()
{
	showItemStatsPanelAction->setChecked(true);
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->statsScrollArea.setVisible(true);
	}
	getActiveMapper().statsEngine.setCurrentlyVisible(true);
}

/**
 * Event handler for the "hide all statistics panels" action in the view menu.
 * 
 * Sets all item-related statistics panels to invisible and sets the corresponding action to
 * unchecked.
 */
void MainWindow::handle_hideAllStatsPanels()
{
	showItemStatsPanelAction->setChecked(false);
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		if (mapper->tabHasBeenOpened(true) && mapper->statsScrollArea.isVisible()) {
			// Save splitter sizes before closing
			QSplitter* const splitter = mapper->tab.findChild<QSplitter*>();
			saveSplitterSizes(*splitter, mapper->statsPanelSplitterSizesSetting);
		}
		mapper->statsScrollArea.setVisible(false);
		mapper->statsEngine.setCurrentlyVisible(false);
	}
}

/**
 * Event handler for the "pin chart ranges" action in the view menu.
 * 
 * Pins or unpins the chart ranges of all item-related statistics panels.
 */
void MainWindow::handle_chartRangesPinnedChanged()
{
	const bool pin = pinStatsRangesAction->isChecked();
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->statsEngine.setRangesPinned(pin);
	}
}

/**
 * Event handler for the "auto-resize columns" action in the view menu.
 * 
 * Resizes all columns in the currently active table to fit their contents.
 */
void MainWindow::handle_autoResizeColumns()
{
	const ItemTypeMapper& mapper = getActiveMapper();
	
	QTableView& tableView = mapper.tableView;
	int numColumns = mapper.compTable.columnCount();
	
	tableView.resizeColumnsToContents();
	for (int i = 0; i < numColumns; i++) {
		if (tableView.columnWidth(i) > 400) tableView.setColumnWidth(i, 400);
	}
}

/**
 * Event handler for the "reset column order" action in the view menu.
 * 
 * Resets the order of columns in the currently active table to their order in the CompositeTable.
 */
void MainWindow::handle_resetColumnOrder()
{
	const ItemTypeMapper& mapper = getActiveMapper();
	
	QHeaderView* header = mapper.tableView.horizontalHeader();
	for (int logicalIndex = 0; logicalIndex < header->count(); logicalIndex++) {
		int currentVisualIndex = header->visualIndex(logicalIndex);
		header->moveSection(currentVisualIndex, logicalIndex);
	}
}

/**
 * Event handler for the "restore hidden columns" action in the view menu.
 * 
 * Resets all columns in the currently active table to being visible.
 */
void MainWindow::handle_restoreHiddenColumns()
{
	const ItemTypeMapper& mapper = getActiveMapper();
	
	QHeaderView* const header = mapper.tableView.horizontalHeader();
	for (int columnIndex = 0; columnIndex < header->count(); columnIndex++) {
		header->setSectionHidden(columnIndex, false);
	}
	mapper.compTable.markAllColumnsUnhidden();
	mapper.compTable.updateBothBuffers();
}

/**
 * Event handler for the 'add custom column' action in the column context menu.
 *
 * Extracts the column index from calling sender().
 */
void MainWindow::handle_addCustomColumn()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;

	ItemTypeMapper& mapper = getActiveMapper();
	
	mapper.tab.openColumnWizard();

	const int logicalIndex = action->data().toInt();
	const int visualIndex = mapper.tableView.horizontalHeader()->visualIndex(logicalIndex) + 1;

	ColumnWizard& wizard = getActiveMapper().columnWizard;

	wizard.visualIndexToUse = visualIndex;
	wizard.restart();
	wizard.show();
}

/**
 * Event handler for the "clear table selection" action in the view menu.
 * 
 * Clears the selection of the currently active table.
 */
void MainWindow::handle_clearTableSelection()
{
	const ItemTypeMapper& mapper = getActiveMapper();
	
	mapper.tableView.selectionModel()->clearSelection();
}



// TOOLS MENU ACTION HANDLERS

/**
 * Event handler for the "auto-fill peak links" action in the tools menu.
 * 
 * Opens the peak link finder dialog.
 */
void MainWindow::handle_findPeakLinks()
{
	PeakLinksDialog* dialog = new PeakLinksDialog(*this, db);
	connect(dialog, &PeakLinksDialog::finished, [=]() { delete dialog; });
	dialog->open();
}

/**
 * Event handler for the "relocate photos" action in the tools menu.
 * 
 * Opens the photo relocation dialog.
 */
void MainWindow::handle_relocatePhotos()
{
	RelocatePhotosDialog* dialog = new RelocatePhotosDialog(*this, db);
	connect(dialog, &RelocatePhotosDialog::finished, [=]() { delete dialog; });
	dialog->open();
}

/**
 * Event handler for the "export data" action in the tools menu.
 * 
 * Opens the photo relocation dialog.
 */
void MainWindow::handle_exportData()
{
	DataExportDialog* dialog = new DataExportDialog(*this, *typesHandler);
	connect(dialog, &DataExportDialog::finished, [=]() { delete dialog; });
	dialog->open();
}



// HELP MENU ACTION HANDLERS

/**
 * Event handler for the "about PAL" action in the help menu.
 * 
 * Opens the about dialog.
 */
void MainWindow::handle_about()
{
	AboutWindow* dialog = new AboutWindow(*this);
	connect(dialog, &AboutWindow::finished, [=]() { delete dialog; });
	dialog->open();
}



// STATE GETTERS

bool MainWindow::isProjectOpen()
{
	return projectOpen;
}

bool MainWindow::getCurrentTabIndex()
{
	return mainAreaTabs->currentIndex();
}

bool MainWindow::getShowItemStatsPanelState()
{
	return showItemStatsPanelAction->isChecked();
}

bool MainWindow::getPinStatRangesState()
{
	return pinStatsRangesAction->isChecked();
}



// CLOSING BEHAVIOUR

/**
 * Event handler for the close event of the main window.
 * 
 * Saves all implicit settings before closing the window.
 * 
 * @param event	The close event.
 */
void MainWindow::closeEvent(QCloseEvent* event)
{
	if (projectOpen) handle_closeDatabase();
	saveGlobalImplicitSettings();
	
	QMainWindow::closeEvent(event);
}

/**
 * Saves current tab index, column widths, sorting and filter bar visiblity.
 */
void MainWindow::saveProjectImplicitSettings()
{
	assert(projectOpen);
	
	db.projectSettings.mainWindow_currentTabIndex.set(*this, mainAreaTabs->currentIndex());
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		// Custom columns
		const QString encodedColumns = mapper->compTable.getEncodedCustomColumns();
		mapper->customColumnsSetting.set(*this, encodedColumns);
		// Implicit column settings
		saveImplicitColumnSettings(*mapper);
		saveSorting(*mapper);
		mapper->filterBar.saveFilters();
		mapper->showFilterBarSetting.set(*this, mapper->filterBarCurrentlySetVisible());
	}
}

/**
 * Saves window position and size.
 */
void MainWindow::saveGlobalImplicitSettings()
{
	bool maximized = windowState() == Qt::WindowMaximized;
	Settings::mainWindow_maximized.set(maximized);
	if (!maximized) Settings::mainWindow_geometry.set(geometry());
	
	Settings::itemStats_pinRanges.set(pinStatsRangesAction->isChecked());
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->showStatsPanelSetting.set(mapper->itemStatsPanelCurrentlySetVisible());
		if (mapper->tabHasBeenOpened(true) && mapper->itemStatsPanelCurrentlySetVisible()) {
			QSplitter* const splitter = mapper->tab.findChild<QSplitter*>();
			saveSplitterSizes(*splitter, mapper->statsPanelSplitterSizesSetting);
		}
	}
}

/**
 * Saves all column-related implicit project settings for the given item type.
 * 
 * @param mapper	The ItemTypeMapper containing the table whose column widths should be saved.
 */
void MainWindow::saveImplicitColumnSettings(const ItemTypeMapper& mapper)
{
	if (!mapper.tabHasBeenOpened(false)) return;	// Only save if table was actually shown
	
	QHeaderView* header = mapper.tableView.horizontalHeader();
	
	QMap<QString, int>	widthsMap;
	QMap<QString, int>	orderMap;
	QMap<QString, bool>	hiddenMap;
	for (int logicalColumnIndex = 0; logicalColumnIndex < mapper.compTable.columnCount(); logicalColumnIndex++) {
		const QString& columnName = mapper.compTable.getColumnAt(logicalColumnIndex).name;
		
		// Hidden status
		const bool hidden = header->isSectionHidden(logicalColumnIndex);
		hiddenMap[columnName] = hidden;
		
		// Column width (not available if column is hidden)
		if (!hidden) {
			int currentColumnWidth = mapper.tableView.columnWidth(logicalColumnIndex);
			if (currentColumnWidth <= 0) {
				qDebug() << "Saving column widths: Couldn't read column width for column" << columnName << "in table" << mapper.compTable.name << "- skipping column";
			} else {
				widthsMap[columnName] = currentColumnWidth;
			}
		}
		
		// Column order
		int visualIndex = header->visualIndex(logicalColumnIndex);
		if (visualIndex < 0) {
			qDebug() << "Saving column order: Couldn't read column order for column" << columnName << "in table" << mapper.compTable.name << "- skipping column";
		} else {
			orderMap[columnName] = visualIndex;
		}
	}
	mapper.columnWidthsSetting	.set(*this, widthsMap);
	mapper.columnOrderSetting	.set(*this, orderMap);
	mapper.hiddenColumnsSetting.set(*this, hiddenMap);
}

/**
 * Saves the sorting of the table for the given item type.
 * 
 * @param mapper	The ItemTypeMapper containing the table whose sorting should be saved.
 */
void MainWindow::saveSorting(const ItemTypeMapper& mapper)
{
	const auto& [column, order] = mapper.compTable.getCurrentSorting();
	QString orderString = order == Qt::DescendingOrder ? "Descending" : "Ascending";
	QString settingValue = column->name + ", " + orderString;
	mapper.sortingSetting.set(*this, settingValue);
}



// LAYOUT CHANGES

/**
 * Event handler for resize events of the main window.
 * 
 * Updates the visibility of the new item buttons in the toolbar, disabling any that don't fit.
 * 
 * @param event	The resize event.
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
	if (!isVisible()) {	// Window is still being initialized
		event->ignore();
		return;
	}
	
	updateTopBarButtonVisibilities();
	event->accept();
}





// GENERAL HELPERS

/**
 * Returns the ItemTypeMapper for the currently active tab, or nullptr if no item tab is active.
 * 
 * @return	The active ItemTypeMapper, or nullptr if no item tab is active.
 */
ItemTypeMapper* MainWindow::getActiveMapperOrNull() const
{
	return typesHandler->getActiveMapperOrNull();
}

/**
 * Returns the ItemTypeMapper for the currently active tab.
 * 
 * @pre An item tab must currently be active.
 * 
 * @return	The active ItemTypeMapper.
 */
ItemTypeMapper& MainWindow::getActiveMapper() const
{
	return typesHandler->getActiveMapper();
}

/**
 * Adds or moves the given filepath to the top of the list of recently opened databases in the
 * settings and updates the recent files menu.
 * 
 * @param filepath	The filepath to add.
 */
void MainWindow::addToRecentFilesList(const QString& filepath)
{
	Settings::lastOpenDatabaseFile.set(filepath);
	
	QStringList recentFiles = Settings::recentDatabaseFiles.get();
	if (recentFiles.contains(filepath)) {
		recentFiles.move(recentFiles.indexOf(filepath), 0);
	} else {
		recentFiles.insert(0, filepath);
		while (recentFiles.size() > 10) recentFiles.removeAt(0);
	}
	
	Settings::recentDatabaseFiles.set(recentFiles);
	updateRecentFilesMenu();
}

/**
 * Sets the window title to the given filepath, or to the default title if no filepath is given.
 * 
 * @param filepath	The filepath to use in the window title.
 */
void MainWindow::setWindowTitleFilename(QString filepath)
{
	QString windowTitle = "PeakAscentLogger";
	if (!filepath.isEmpty()) {
		QString filename = QFileInfo(filepath).fileName();
		windowTitle = filename + "  –  " + windowTitle;
	}
	setWindowTitle(windowTitle);
}

/**
 * Temporarily sets the status bar message to the given message.
 * 
 * @param content	The message to show.
 */
void MainWindow::setStatusLine(QString content)
{
	statusbar->showMessage(content, DYNAMIC_STATUS_MESSAGE_DURATION_SEC * 1000);
}
