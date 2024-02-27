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
	openRecentActions(QList<QAction*>()),
	columnContextMenu(QMenu(this)), columnContextMenuHideColumnAction(nullptr), columnContextMenuRestoreColumnMenu(nullptr),
	tableContextMenu(QMenu(this)), tableContextMenuOpenAction(nullptr), tableContextMenuDuplicateAction(nullptr),
	shortcuts(QList<QShortcut*>()),
	statusBarTableSizeLabel(new QLabel(statusbar)),
	statusBarFiltersLabel(new QLabel(statusbar)),
	typesHandler(nullptr),
	generalStatsEngine(GeneralStatsEngine(db, &statisticsTabLayout))
{
	setupUi(this);
	createTypesHandler();
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
	
	ascentFilterBar->supplyPointers(this, &db, (CompositeAscentsTable*) typesHandler->get(ItemTypeAscent).compTable);
	
	
	connectUI();
	setupTableTabs();
	generalStatsEngine.setupStatsTab();
	initColumnContextMenu();
	initTableContextMenuAndShortcuts();
	updateItemCountDisplays(true);
	updateRecentFilesMenu();
	
	// Open database
	QString lastOpen = Settings::lastOpenDatabaseFile.get();
	if (!lastOpen.isEmpty() && QFile(lastOpen).exists()) {
		attemptToOpenFile(lastOpen);
	}
	
	
	updateTableContextMenuIcons();
}

/**
 * Destroys the MainWindow.
 */
MainWindow::~MainWindow()
{
	delete columnContextMenuRestoreColumnMenu;
	qDeleteAll(shortcuts);
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
			{ItemTypeAscent,	TypeMapperPointers{ascentsTab,		ascentsTableView,	ascentsStatsScrollArea,		newAscentAction,	newAscentButton}	},
			{ItemTypePeak,		TypeMapperPointers{peaksTab,		peaksTableView,		peaksStatsScrollArea,		newPeakAction,		newPeakButton}		},
			{ItemTypeTrip,		TypeMapperPointers{tripsTab,		tripsTableView,		tripsStatsScrollArea,		newTripAction,		newTripButton}		},
			{ItemTypeHiker,		TypeMapperPointers{hikersTab,		hikersTableView,	hikersStatsScrollArea,		newHikerAction,		newHikerButton}		},
			{ItemTypeRegion,	TypeMapperPointers{regionsTab,		regionsTableView,	regionsStatsScrollArea,		newRegionAction,	newRegionButton}	},
			{ItemTypeRange,		TypeMapperPointers{rangesTab,		rangesTableView,	rangesStatsScrollArea,		newRangeAction,		newRangeButton}		},
			{ItemTypeCountry,	TypeMapperPointers{countriesTab,	countriesTableView,	countriesStatsScrollArea,	newCountryAction,	newCountryButton}	}
		}
	);
}

/**
 * Adds standard icons to some menu items.
 */
void MainWindow::setupMenuIcons()
{
	newDatabaseAction			->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
	openDatabaseAction			->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	openRecentMenu				->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	saveDatabaseAsAction		->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	closeDatabaseAction			->setIcon(style()->standardIcon(QStyle::SP_TabCloseButton));
	projectSettingsAction		->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	settingsAction				->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	// View menu
	// showItemStatsPanelAction is checkable
	showAllStatsPanelsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	hideAllStatsPanelsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	// pinStatsRangesAction is checkable
	// showFiltersAction is checkable
	autoResizeColumnsAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	resetColumnOrderAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	restoreHiddenColumnsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	// New menu: using own icons
	// Tools menu
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
	connect(clearTableSelectionAction,		&QAction::triggered,			this,	&MainWindow::handle_clearTableSelection);
	// Menu "New"
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		auto newFunction = [this, mapper] () {
			newItem(*mapper);
		};
		
		connect(mapper->newItemAction,		&QAction::triggered,			this,	newFunction);
		// Big new item buttons
		if (mapper->newItemButton) {
			connect(mapper->newItemButton,	&QPushButton::clicked,			this,	newFunction);
		}
	}
	// Menu "Tools"
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
		connect(mapper->tableView,			&QTableView::doubleClicked,		this,	handlerFunction);
		connect(mapper->compTable,			&CompositeTable::wasResorted,	this,	&MainWindow::scrollToTopAfterSorting);
	}
}

/**
 * Connects each table view to the underlying CompositeTable and to the table context menu, as well
 * as set up the item statistics panels.
 */
void MainWindow::setupTableTabs()
{
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		// Set model
		mapper->tableView->setModel(mapper->compTable);
		mapper->compTable->setUpdateImmediately(mapper->type == mainAreaTabs->currentIndex());
		
		// Enable column header reordering
		mapper->tableView->horizontalHeader()->setSectionsMovable(true);
		
		// Enable context menus
		mapper->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(mapper->tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MainWindow::handle_rightClickOnColumnHeader);
		connect(mapper->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClickInTable);
		
		// Connect selection change listener
		connect(mapper->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::handle_tableSelectionChanged);
		
		// Stats visibility
		const bool statsEnabled = mapper->showStatsPanelSetting->get();
		mapper->statsScrollArea->setVisible(statsEnabled);
		// Restore splitter sizes
		QSplitter* const splitter = mapper->tab->findChild<QSplitter*>();
		splitter->setStretchFactor(0, 3);
		splitter->setStretchFactor(1, 1);
		restoreSplitterSizes(*splitter, *mapper->statsPanelSplitterSizesSetting);
		
		// Setup stats panels
		mapper->statsEngine->setupStatsPanel();
		mapper->statsEngine->setCurrentlyVisible(false);
		mapper->statsEngine->setRangesPinned(pinStatsRangesAction->isChecked());
	}
}

/**
 * Restores the column widths for the table view specified by the given ItemTypeMapper.
 * 
 * @param mapper	The ItemTypeMapper for the table view whose column widths should be restored.
 */
void MainWindow::restoreColumnWidths(const ItemTypeMapper* const mapper)
{
	QSet<QString> columnNameSet = mapper->compTable->getNormalColumnNameSet();
	if (mapper->columnWidthsSetting->nonePresent(columnNameSet)) return;	// Only restore if any widths are in the settings
	
	const QSet<QString> normalColumnNames = mapper->compTable->getNormalColumnNameSet();
	const QMap<QString, int> columnWidthMap = mapper->columnWidthsSetting->get(normalColumnNames);
	
	// Restore column widths
	for (int columnIndex = 0; columnIndex < mapper->compTable->getNumberOfNormalColumns(); columnIndex++) {
		const QString& columnName = mapper->compTable->getColumnAt(columnIndex).name;
		int columnWidth = columnWidthMap[columnName];
		if (columnWidth < 1) {
			columnWidth = mapper->tableView->horizontalHeader()->sizeHintForColumn(columnIndex);
		}
		mapper->tableView->setColumnWidth(columnIndex, columnWidth);
	}
}

/**
 * Restores the column order for the table view specified by the given ItemTypeMapper.
 * 
 * @param mapper	The ItemTypeMapper for the table view whose column order should be restored.
 */
void MainWindow::restoreColumnOrder(const ItemTypeMapper* const mapper)
{
	QSet<QString> columnNameSet = mapper->compTable->getNormalColumnNameSet();
	if (mapper->columnOrderSetting->nonePresent(columnNameSet)) return;	// Only restore if any columns are in the settings
	
	const QSet<QString> normalColumnNames = mapper->compTable->getNormalColumnNameSet();
	const QMap<QString, int> columnOrderMap = mapper->columnOrderSetting->get(normalColumnNames);
	// Sort by visual index
	QList<QPair<const CompositeColumn*, int>> columnOrderList = QList<QPair<const CompositeColumn*, int>>();
	for (const QPair<QString, int>& columnOrderPair : columnOrderMap.asKeyValueRange()) {
		if (columnOrderPair.second < 0) continue;	// Visual index invalid, ignore column
		const CompositeColumn* column = mapper->compTable->getColumnByNameOrNull(columnOrderPair.first);
		if (!column) continue;
		columnOrderList.append({column, columnOrderPair.second});
	}
	auto comparator = [](const QPair<const CompositeColumn*, int>& pair1, const QPair<const CompositeColumn*, int>& pair2) {
		return pair1.second < pair2.second;
	};
	std::sort(columnOrderList.begin(), columnOrderList.end(), comparator);
	
	// Restore column order
	QHeaderView* header = mapper->tableView->horizontalHeader();
	for (int visualIndex = 0; visualIndex < columnOrderList.size(); visualIndex++) {
		const CompositeColumn& column = *columnOrderList.at(visualIndex).first;
		int logicalIndex = column.getIndex();
		int currentVisualIndex = header->visualIndex(logicalIndex);
		header->moveSection(currentVisualIndex, visualIndex);
	}
}

/**
 * Restores the column hidden states for the table view specified by the given ItemTypeMapper.
 * 
 * @param mapper	The ItemTypeMapper for the table view whose column hidden statuses should be restored.
 */
void MainWindow::restoreColumnHiddenStatus(const ItemTypeMapper* const mapper)
{
	QSet<QString> columnNameSet = mapper->compTable->getNormalColumnNameSet();
	if (mapper->hiddenColumnsSetting->nonePresent(columnNameSet)) return;	// Only restore if any column are in the settings
	
	const QSet<QString> normalColumnNames = mapper->compTable->getNormalColumnNameSet();
	const QMap<QString, bool> columnHiddenMap = mapper->hiddenColumnsSetting->get(normalColumnNames);
	
	// Restore column hidden status
	for (int columnIndex = 0; columnIndex < mapper->compTable->getNumberOfNormalColumns(); columnIndex++) {
		const QString& columnName = mapper->compTable->getColumnAt(columnIndex).name;
		bool storedColumnHiddenStatus = columnHiddenMap[columnName];
		if (!storedColumnHiddenStatus) continue;
		mapper->tableView->horizontalHeader()->setSectionHidden(columnIndex, true);
		mapper->compTable->markColumnHidden(columnIndex);
	}
}

/**
 * Sets the sorting for the table view specified by the given ItemTypeMapper to either the
 * remembered sorting or, if that is not present or disabled, to the default sorting.
 * 
 * @param mapper	The ItemTypeMapper for the table view whose sorting should be set.
 */
void MainWindow::setSorting(const ItemTypeMapper* const mapper)
{
	SortingPass sorting = mapper->compTable->getDefaultSorting();
	bool sortingSettingValid = true;
	
	while (Settings::rememberSorting.get() && mapper->sortingSetting->present()) {
		sortingSettingValid = false;
		
		QStringList saved = mapper->sortingSetting->get().split(",");
		if (saved.size() != 2) break;
		
		const CompositeColumn* column = mapper->compTable->getColumnByNameOrNull(saved.at(0).trimmed());
		if (!column) break;
		
		bool ascending = saved.at(1).trimmed().compare("Descending", Qt::CaseInsensitive) != 0;
		Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
		
		sorting.column = column;
		sorting.order = order;
		sortingSettingValid = true;
		break;
	}
	mapper->tableView->sortByColumn(sorting.column->getIndex(), sorting.order);
	
	if (!sortingSettingValid) mapper->sortingSetting->clear(*this);
}


/**
 * Initializes the column context menu.
 */
void MainWindow::initColumnContextMenu()
{
	// Context menu
	columnContextMenuHideColumnAction = columnContextMenu.addAction(tr("Hide this column"));
	columnContextMenu.addSeparator();
	columnContextMenuRestoreColumnMenu = columnContextMenu.addMenu(tr("Restore hidden column"));
	
	connect(columnContextMenuHideColumnAction, &QAction::triggered, this, &MainWindow::handle_hideColumn);
}

/**
 * Initializes the table context menu and the keyboard shortcuts for the table views.
 */
void MainWindow::initTableContextMenuAndShortcuts()
{
	QKeySequence openKeySequence		= QKeySequence(Qt::Key_Return);
	QKeySequence editKeySequence		= QKeySequence(Qt::CTRL | Qt::Key_Return);
	QKeySequence duplicateKeySequence	= QKeySequence::Copy;
	QKeySequence deleteKeySequence		= QKeySequence::Delete;
	
	// Context menu
	QAction* openAction			= tableContextMenu.addAction(tr("View..."),						openKeySequence);
	tableContextMenu.addSeparator();
	QAction* editAction			= tableContextMenu.addAction(tr("Edit..."),						editKeySequence);
	QAction* duplicateAction	= tableContextMenu.addAction(tr("Edit as new duplicate..."),	duplicateKeySequence);
	tableContextMenu.addSeparator();
	QAction* deleteAction		= tableContextMenu.addAction(tr("Delete"),						deleteKeySequence);
	// Store actions for open and duplicate (for disbling them where they're not needed)
	tableContextMenuOpenAction		= openAction;
	tableContextMenuEditAction		= editAction;
	tableContextMenuDuplicateAction	= duplicateAction;
	tableContextMenuDeleteAction	= deleteAction;
	
	// Set icons
	openAction->setIcon(QIcon(":/icons/ascent_viewer.svg"));
	deleteAction->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	
	connect(openAction,			&QAction::triggered, this, &MainWindow::viewSelectedItem);
	connect(editAction,			&QAction::triggered, this, &MainWindow::editSelectedItems);
	connect(duplicateAction,	&QAction::triggered, this, &MainWindow::duplicateAndEditSelectedItem);
	connect(deleteAction,		&QAction::triggered, this, &MainWindow::deleteSelectedItems);
	
	// Keyboard shortcuts
	QList<QTableView*> tableViews = mainAreaTabs->findChildren<QTableView*>();
	for (QTableView* const tableView : tableViews) {
		QShortcut* openShortcut			= new QShortcut(openKeySequence,		tableView);
		QShortcut* editShortcut			= new QShortcut(editKeySequence,		tableView);
		QShortcut* duplicateShortcut	= new QShortcut(duplicateKeySequence,	tableView);
		QShortcut* deleteShortcut		= new QShortcut(deleteKeySequence,		tableView);
		
		shortcuts.append(openShortcut);
		shortcuts.append(editShortcut);
		shortcuts.append(duplicateShortcut);
		shortcuts.append(deleteShortcut);
		
		auto handle_enter = [this]() {
			const ItemTypeMapper* activeMapper = getActiveMapperOrNull();
			if (!activeMapper) return;
			if (activeMapper->type == ItemTypeAscent) return viewSelectedItem();
			return editSelectedItems();
		};
		connect(openShortcut,		&QShortcut::activated, this, handle_enter);
		connect(editShortcut,		&QShortcut::activated, this, &MainWindow::editSelectedItems);
		connect(duplicateShortcut,	&QShortcut::activated, this, &MainWindow::duplicateAndEditSelectedItem);
		connect(deleteShortcut,		&QShortcut::activated, this, &MainWindow::deleteSelectedItems);
	}
}

/**
 * Sets the icons for the table context menu actions to the icon of the currently selected table.
 * 
 * This method has to be called whenever the currently selected table changes because the context
 * menu actions are shared between all tables.
 */
void MainWindow::updateTableContextMenuIcons()
{
	const ItemTypeMapper* const activeMapperOrNull = getActiveMapperOrNull();
	if (!activeMapperOrNull) return;
	QIcon icon = QIcon(":/icons/" + activeMapperOrNull->name + ".svg");
	tableContextMenuEditAction->setIcon(icon);
	tableContextMenuDuplicateAction->setIcon(icon);
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
		updateFilters();
		
		// Restore project-specific implicit settings:
		// Filter bar
		showFiltersAction->setChecked(db.projectSettings.mainWindow_showFilterBar.get(this));
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
			activeMapper->statsEngine->setCurrentlyVisible(true, true);
		}
		generalStatsEngine.setCurrentlyVisible(!activeMapper);
		
		for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
			// Column widths
			if (Settings::rememberColumnWidths.get()) {
				restoreColumnWidths(mapper);
			}
			// Column order
			if (Settings::rememberColumnOrder.get()) {
				restoreColumnOrder(mapper);
			}
			// Hidden columns
			if (Settings::rememberHiddenColumns.get()) {
				restoreColumnHiddenStatus(mapper);
			}
			// Sortings
			setSorting(mapper);
		}
		
		// Build buffers and update size info
		initCompositeBuffers();
		projectOpen = true;
		
		updateItemCountDisplays();
		handle_tableSelectionChanged();
		
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
	progress.setMinimumDuration(250);
	progress.setLabel(new QLabel(tr("Preparing tables..."), &progress));
	
	bool prepareAll = !Settings::onlyPrepareActiveTableOnStartup.get();
	const ItemTypeMapper* const activeMapper = getActiveMapperOrNull();
	const QTableView* const currentTableView = activeMapper ? activeMapper->tableView : nullptr;
	
	int numCells = 0;
	if (prepareAll) {
		for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
			numCells += mapper->compTable->getNumberOfCellsToInit();
		}
	} else {
		if (activeMapper) {
			numCells += activeMapper->compTable->getNumberOfCellsToInit();
		}
	}
	progress.setMinimum(0);
	progress.setMaximum(numCells);
	progress.setValue(0);
	
	QSet<Filter> ascentFilters = QSet<Filter>();
	if (Settings::rememberFilters.get()) {
		ascentFilters = ascentFilterBar->parseFiltersFromProjectSettings();
		ascentFilterBar->insertFiltersIntoUI(ascentFilters);
		if (!ascentFilters.isEmpty()) {
			showFiltersAction->setChecked(true);
		}
	}
	typesHandler->get(ItemTypeAscent).compTable->setInitialFilters(ascentFilters);
	
	for (ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		progress.setLabelText(tr("Preparing table %1...").arg(mapper->baseTable.uiName));
		
		bool isOpen = mapper->tableView == currentTableView;
		bool prepareThisTable = prepareAll || isOpen;
		
		QSet<QString> columnNameSet = mapper->compTable->getNormalColumnNameSet();
		bool autoResizeColumns = !Settings::rememberColumnWidths.get() || mapper->columnWidthsSetting->nonePresent(columnNameSet);
		
		// Collect buffer initialization parameters
		QProgressDialog* updateProgress = prepareThisTable ? &progress : nullptr;
		bool deferCompute = !prepareThisTable;
		QTableView* tableToAutoResizeAfterCompute = autoResizeColumns ? mapper->tableView : nullptr;
		
		mapper->compTable->initBuffer(updateProgress, deferCompute, tableToAutoResizeAfterCompute);
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
	int currentTabIndex = mainAreaTabs->currentIndex();
	bool statsTabOpen = currentTabIndex == mainAreaTabs->indexOf(statisticsTab);
	bool ascentsTabOpen = currentTabIndex == mainAreaTabs->indexOf(ascentsTab);
	
	saveDatabaseAsAction		->setEnabled(enabled);
	closeDatabaseAction			->setEnabled(enabled);
	projectSettingsAction		->setEnabled(enabled);
	viewMenu					->setEnabled(enabled && !statsTabOpen);
	newMenu						->setEnabled(enabled);
	toolsMenu					->setEnabled(enabled);
	
	showFiltersAction			->setVisible(enabled && ascentsTabOpen);
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->newItemButton->setEnabled(enabled);
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
			int displayed = activeMapper->compTable->rowCount();
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
		int numFiltersApplied = activeMapper->compTable->getCurrentFilters().size();
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
	const int numAscentsShown = typesHandler->get(ItemTypeAscent).compTable->rowCount();
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
	handle_tableSelectionChanged();
}



// EXECUTE USER COMMANDS

/**
 * Opens the item at the primary selected row in the currently active table.
 */
void MainWindow::viewSelectedItem()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const BufferRowIndex primaryBufferRow = getSelectedRows(activeMapper).second;
	if (primaryBufferRow.isInvalid()) return;
	
	switch (activeMapper.type) {
	case ItemTypeAscent: {
		const ViewRowIndex markedViewRow = activeMapper.compTable->findViewRowIndexForBufferRow(primaryBufferRow);
		AscentViewer(this, db, typesHandler, markedViewRow).exec();
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
	BufferRowIndex newBufferRowIndex = mapper.openNewItemDialogAndStoreMethod(*this, *this, db);
	if (newBufferRowIndex.isInvalid()) return;
	
	setStatusLine(tr("Saved new %1.").arg(mapper.baseTable.getItemNameSingularLowercase()));
	performUpdatesAfterUserAction(mapper, true, newBufferRowIndex);
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
	const BufferRowIndex primaryBufferRow = getSelectedRows(activeMapper).second;
	if (primaryBufferRow.isInvalid()) return;
	
	BufferRowIndex newBufferRowIndex = activeMapper.openDuplicateItemDialogAndStoreMethod(*this, *this, db, primaryBufferRow);
	if (newBufferRowIndex.isInvalid()) return;
	
	setStatusLine(tr("Saved new %1.").arg(activeMapper.baseTable.getItemNameSingularLowercase()));
	performUpdatesAfterUserAction(activeMapper, true, newBufferRowIndex);
}

/**
 * Opens a dialog for editing the items at the selected rows in the currently active table.
 * 
 * If the items were edited, performs the necessary updates to the UI.
 */
void MainWindow::editSelectedItems()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const QPair<QSet<BufferRowIndex>, BufferRowIndex> selectedAndMarkedBufferRows = getSelectedRows(activeMapper);
	const QSet<BufferRowIndex>& selectedBufferRows = selectedAndMarkedBufferRows.first;
	const BufferRowIndex markedBufferRow = selectedAndMarkedBufferRows.second;
	if (selectedBufferRows.isEmpty()) return;
	
	if (selectedBufferRows.size() < 2) {
		const bool changesMade = activeMapper.openEditItemDialogAndStoreMethod(*this, *this, db, markedBufferRow);
		if (!changesMade) return;
		
		setStatusLine(tr("Saved changes in %1.").arg(activeMapper.baseTable.getItemNameSingularLowercase()));
	}
	else {
		const bool changesMade = activeMapper.openMultiEditItemDialogAndStoreMethod(*this, *this, db, selectedBufferRows, markedBufferRow);
		if (!changesMade) return;
		
		setStatusLine(tr("Saved changes in %1 %2.").arg(selectedBufferRows.size()).arg(activeMapper.baseTable.getItemNamePluralLowercase()));
	}
	performUpdatesAfterUserAction(activeMapper, false, markedBufferRow);
}

/**
 * Opens a dialog for deleting the selected items in the currently active table.
 * 
 * If the item was deleted, performs the necessary updates to the UI.
 */
void MainWindow::deleteSelectedItems()
{
	const ItemTypeMapper& activeMapper = getActiveMapper();
	const QSet<BufferRowIndex> selectedBufferRows = getSelectedRows(activeMapper).first;
	if (selectedBufferRows.isEmpty()) return;
	
	QSet<ViewRowIndex> selectedViewRowIndices = QSet<ViewRowIndex>();
	for (const QModelIndex& index : getActiveMapper().tableView->selectionModel()->selectedRows()) {
		selectedViewRowIndices += ViewRowIndex(index.row());
	}
	if (selectedViewRowIndices.isEmpty()) return;
	
	const bool deleted = activeMapper.openDeleteItemsDialogAndExecuteMethod(*this, *this, db, selectedBufferRows);
	if (!deleted) return;

	if (selectedBufferRows.size() == 1) {
		setStatusLine(tr("Deleted %1.").arg(activeMapper.baseTable.getItemNameSingularLowercase()));
	} else {
		setStatusLine(tr("Deleted %1 %2.").arg(selectedBufferRows.size()).arg(activeMapper.baseTable.getItemNamePluralLowercase()));
	}
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
		QPushButton* const button = mapper->newItemButton;
		const bool buttonVisible = button->x() + button->width() < leftmostCounterLabel->x() - 40;
		button->setVisible(buttonVisible);
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
		ViewRowIndex viewRowToSelectIndex = mapper.compTable->findViewRowIndexForBufferRow(bufferRowToSelectIndex);
		updateSelectionAfterUserAction(mapper, viewRowToSelectIndex);
	}
	// Update table size info
	if (numberOfEntriesChanged) updateItemCountDisplays();
	// Update filters
	updateFilters();
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
	
	QTableView* const tableView = activeMapper->tableView;
	const int horizontalScroll = tableView->horizontalScrollBar()->value();
	tableView->scrollToTop();
	tableView->horizontalScrollBar()->setValue(horizontalScroll);
}

/**
 * Updates elements of the ascent filter bar which depend on contents of the composite tables.
 * 
 * @param mapper The ItemTypeMapper for the table that was changed.
 */
void MainWindow::updateFilters(const ItemTypeMapper* mapper)
{
	if (!mapper || mapper->type == ItemTypeRange)	ascentFilterBar->updateRangeCombo();
	if (!mapper || mapper->type == ItemTypeHiker)	ascentFilterBar->updateHikerCombo();
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
	QModelIndex modelIndex = mapper.compTable->index(viewRowIndex.get(), 0);
	mapper.tableView->setCurrentIndex(modelIndex);
	mapper.tableView->scrollTo(modelIndex);
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
	progress.setMinimumDuration(500);
	
	for (ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		if (mapper == activeMapper) continue;
		
		mapper->compTable->setUpdateImmediately(false);
		mapper->statsEngine->setCurrentlyVisible(false);
	}
	
	if (activeMapper) {
		// Make sure active table is up to date
		progress.setMaximum(activeMapper->compTable->getNumberOfCellsToUpdate());
		activeMapper->compTable->setUpdateImmediately(true, &progress);
		activeMapper->openingTab();
		
		// Item stats panel visibility
		const bool statsShown = activeMapper->itemStatsPanelCurrentlySetVisible();
		showItemStatsPanelAction->setChecked(statsShown);
		activeMapper->statsEngine->setCurrentlyVisible(statsShown, true);
	}
	generalStatsEngine.setCurrentlyVisible(!activeMapper);
	
	updateTableContextMenuIcons();
	handle_tableSelectionChanged();
	
	setUIEnabled(true);
}

/**
 * Event handler for changes in which rows of the active table view are selected.
 * 
 * Collects selected rows and updates the item statistics panel.
 */
void MainWindow::handle_tableSelectionChanged()
{
	bool statsPanelShown = showItemStatsPanelAction->isChecked();
	if (!projectOpen || !statsPanelShown) return;
	
	const ItemTypeMapper* const activeMapper = getActiveMapperOrNull();
	if (!activeMapper) return;
	
	const QItemSelection selection = activeMapper->tableView->selectionModel()->selection();
	QSet<BufferRowIndex> selectedBufferRows = QSet<BufferRowIndex>();
	if (selection.isEmpty()) {
		// Instead of showing an empty chart, show chart for all rows (except filtered out)
		int numRowsShown = activeMapper->compTable->rowCount();
		for (ViewRowIndex viewIndex = ViewRowIndex(0); viewIndex.isValid(numRowsShown); viewIndex++) {
			BufferRowIndex bufferIndex = activeMapper->compTable->getBufferRowIndexForViewRow(viewIndex);
			selectedBufferRows.insert(bufferIndex);
		}
	}
	else {
		// One or more rows selected, find their buffer indices
		QSet<ViewRowIndex> selectedViewRows = QSet<ViewRowIndex>();
		for (const QItemSelectionRange& range : selection) {
			for (const QModelIndex& index : range.indexes()) {
				selectedViewRows.insert(ViewRowIndex(index.row()));
			}
		}
		for (const ViewRowIndex& viewIndex : selectedViewRows) {
			BufferRowIndex bufferIndex = activeMapper->compTable->getBufferRowIndexForViewRow(viewIndex);
			selectedBufferRows.insert(bufferIndex);
		}
	}
	
	const bool allSelected = activeMapper->compTable->rowCount() == selectedBufferRows.size();
	activeMapper->statsEngine->setStartBufferRows(selectedBufferRows, allSelected);
}

/**
 * Event handler for right clicks on the column header area of the active table view.
 * 
 * Prepares and opens the column context menu at the given position.
 * 
 * @param pos	The position of the right click in the viewport of the horizontal table view header.
 */
void MainWindow::handle_rightClickOnColumnHeader(QPoint pos)
{
	const ItemTypeMapper& mapper = getActiveMapper();
	
	// Get index of clicked column
	QHeaderView* header = mapper.tableView->horizontalHeader();
	int logicalIndexClicked = header->logicalIndexAt(pos);
	if (logicalIndexClicked < 0) return;
	
	columnContextMenuHideColumnAction->setData(logicalIndexClicked);
	
	// Repopulate 'restore column' submenu
	int visibleColumns = 0;
	columnContextMenuRestoreColumnMenu->clear();
	for (int logicalIndex = 0; logicalIndex < header->count(); logicalIndex++) {
		if (!header->isSectionHidden(logicalIndex)) {
			visibleColumns++;
			continue;
		}
		
		const QString& columnName = mapper.compTable->getColumnAt(logicalIndex).uiName;
		QAction* restoreColumnAction = columnContextMenuRestoreColumnMenu->addAction(columnName);
		restoreColumnAction->setData(logicalIndex);
		connect(restoreColumnAction, &QAction::triggered, this, &MainWindow::handle_unhideColumn);
	}
	columnContextMenuRestoreColumnMenu->setEnabled(!columnContextMenuRestoreColumnMenu->isEmpty());
	
	columnContextMenuHideColumnAction->setEnabled(visibleColumns > 1);
	
	// Show contet menu
	columnContextMenu.popup(header->viewport()->mapToGlobal(pos));
}

/**
 * Event handler for right clicks in the cell area of the active table view.
 * 
 * Prepares and opens the table context menu at the given position.
 * 
 * @param pos	The position of the right click in the viewport of the table view.
 */
void MainWindow::handle_rightClickInTable(QPoint pos)
{
	QTableView& currentTableView = *getActiveMapper().tableView;
	QModelIndex index = currentTableView.indexAt(pos);
	if (!index.isValid()) return;
	
	const bool singleRowSelected = currentTableView.selectionModel()->selectedRows().size() == 1;
	const bool viewableItemTable = &currentTableView == ascentsTableView;
	const bool duplicatableItemTable = &currentTableView == ascentsTableView || &currentTableView == peaksTableView;
	
	tableContextMenuOpenAction		->setVisible(singleRowSelected && viewableItemTable);
	tableContextMenuDuplicateAction	->setVisible(singleRowSelected && duplicatableItemTable);
	
	QString deleteString = tr("Delete") + (Settings::confirmDelete.get() ? "..." : "");
	tableContextMenuDeleteAction->setText(deleteString);
	
	tableContextMenu.popup(currentTableView.viewport()->mapToGlobal(pos));
}



// COLUMN CONTEXT MENU ACTION HANDLERS

/**
 * Event handler for the 'hide column' action in the column context menu.
 * 
 * Extracts the column index from calling sender().
 */
void MainWindow::handle_hideColumn()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;
	
	int logicalIndex = action->data().toInt();
	
	const ItemTypeMapper& mapper = getActiveMapper();
	
	mapper.tableView->horizontalHeader()->setSectionHidden(logicalIndex, true);
	mapper.compTable->markColumnHidden(logicalIndex);
	
}

/**
 * Event handler for any of the 'unhide column' actions in the column context menu.
 * 
 * Extracts the column index from calling sender().
 */
void MainWindow::handle_unhideColumn()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;
	
	int logicalIndex = action->data().toInt();
	
	const ItemTypeMapper& mapper = getActiveMapper();
	
	mapper.tableView->horizontalHeader()->setSectionHidden(logicalIndex, false);
	mapper.compTable->markColumnUnhidden(logicalIndex);
	mapper.compTable->updateBothBuffers();
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
	
	updateFilters();
	updateItemCountDisplays();
	handle_tableSelectionChanged();
	setUIEnabled(true);
	
	addToRecentFilesList(filepath);
	
	if (Settings::openProjectSettingsOnNewDatabase.get()) {
		ProjectSettingsWindow(*this, *this, db, true).exec();
	}
	
	getActiveMapper().openingTab();
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
	ascentFilterBar->resetUI();
	db.reset();
	projectOpen = false;
	updateFilters();
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->compTable->resetBuffer();
		mapper->statsEngine->resetStatsPanel();
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
	ProjectSettingsWindow(*this, *this, db).exec();
}

/**
 * Event handler for the "settings" action in the file menu.
 * 
 * Opens the settings dialog.
 */
void MainWindow::handle_openSettings()
{
	SettingsWindow(*this).exec();
}



// VIEW MENU ACTION HANDLERS

/**
 * Event handler for the "show filters" action in the view menu.
 * 
 * Shows or hides the ascent filter bar.
 */
void MainWindow::handle_showFiltersChanged()
{
	bool showFilters = showFiltersAction->isChecked();
	ascentFilterBar->setVisible(showFilters);
	if (projectOpen && !showFilters) ascentFilterBar->handle_clearFilters();
}

/**
 * Event handler for the "show statistics panel" action in the view menu.
 * 
 * Shows or hides the active item-related statistics panel.
 */
void MainWindow::handle_showStatsPanelChanged()
{
	if (!projectOpen) return;
	ItemTypeMapper& mapper = getActiveMapper();
	
	if (mapper.statsScrollArea->isVisible()) {
		// Save splitter sizes before closing
		QSplitter* const splitter = mapper.tab->findChild<QSplitter*>();
		saveSplitterSizes(*splitter, *mapper.statsPanelSplitterSizesSetting);
	}
	
	const bool showStatsPanel = showItemStatsPanelAction->isChecked();
	mapper.statsScrollArea->setVisible(showStatsPanel);
	mapper.statsEngine->setCurrentlyVisible(showStatsPanel);
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
		mapper->statsScrollArea->setVisible(true);
	}
	getActiveMapper().statsEngine->setCurrentlyVisible(true);
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
		if (mapper->tabHasBeenOpened(true) && mapper->statsScrollArea->isVisible()) {
			// Save splitter sizes before closing
			QSplitter* const splitter = mapper->tab->findChild<QSplitter*>();
			saveSplitterSizes(*splitter, *mapper->statsPanelSplitterSizesSetting);
		}
		mapper->statsScrollArea->setVisible(false);
		mapper->statsEngine->setCurrentlyVisible(false);
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
		mapper->statsEngine->setRangesPinned(pin);
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
	
	QTableView* tableView = mapper.tableView;
	int numColumns = mapper.compTable->columnCount();
	
	tableView->resizeColumnsToContents();
	for (int i = 0; i < numColumns; i++) {
		if (tableView->columnWidth(i) > 400) tableView->setColumnWidth(i, 400);
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
	
	QHeaderView* header = mapper.tableView->horizontalHeader();
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
	
	QHeaderView* const header = mapper.tableView->horizontalHeader();
	for (int columnIndex = 0; columnIndex < header->count(); columnIndex++) {
		header->setSectionHidden(columnIndex, false);
	}
	mapper.compTable->markAllColumnsUnhidden();
	mapper.compTable->updateBothBuffers();
}

/**
 * Event handler for the "clear table selection" action in the view menu.
 * 
 * Clears the selection of the currently active table.
 */
void MainWindow::handle_clearTableSelection()
{
	const ItemTypeMapper& mapper = getActiveMapper();
	
	mapper.tableView->selectionModel()->clearSelection();
}



// TOOLS MENU ACTION HANDLERS

/**
 * Event handler for the "relocate photos" action in the tools menu.
 * 
 * Opens the photo relocation dialog.
 */
void MainWindow::handle_relocatePhotos()
{
	RelocatePhotosDialog(*this, db).exec();
}

/**
 * Event handler for the "export data" action in the tools menu.
 * 
 * Opens the photo relocation dialog.
 */
void MainWindow::handle_exportData()
{
	DataExportDialog(*this, *typesHandler).exec();
}



// HELP MENU ACTION HANDLERS

/**
 * Event handler for the "about PAL" action in the help menu.
 * 
 * Opens the about dialog.
 */
void MainWindow::handle_about()
{
	AboutWindow(*this).exec();
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
	
	db.projectSettings.mainWindow_currentTabIndex	.set(*this, mainAreaTabs->currentIndex());
	db.projectSettings.mainWindow_showFilterBar		.set(*this, showFiltersAction->isChecked());
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		saveImplicitColumnSettings(mapper);
		saveSorting(mapper);
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
		mapper->showStatsPanelSetting->set(mapper->itemStatsPanelCurrentlySetVisible());
		if (mapper->tabHasBeenOpened(true) && mapper->itemStatsPanelCurrentlySetVisible()) {
			QSplitter* const splitter = mapper->tab->findChild<QSplitter*>();
			saveSplitterSizes(*splitter, *mapper->statsPanelSplitterSizesSetting);
		}
	}
}

/**
 * Saves all column-related implicit project settings for the given item type.
 * 
 * @param mapper	The ItemTypeMapper containing the table whose column widths should be saved.
 */
void MainWindow::saveImplicitColumnSettings(const ItemTypeMapper* const mapper)
{
	if (!mapper->tabHasBeenOpened(false)) return;	// Only save if table was actually shown
	QHeaderView* header = mapper->tableView->horizontalHeader();
	
	QMap<QString, int>	widthsMap;
	QMap<QString, int>	orderMap;
	QMap<QString, bool>	hiddenMap;
	for (int logicalColumnIndex = 0; logicalColumnIndex < mapper->compTable->columnCount(); logicalColumnIndex++) {
		const QString& columnName = mapper->compTable->getColumnAt(logicalColumnIndex).name;
		
		// Hidden status
		const bool hidden = header->isSectionHidden(logicalColumnIndex);
		hiddenMap[columnName] = hidden;
		
		// Column width (not available if column is hidden)
		if (!hidden) {
			int currentColumnWidth = mapper->tableView->columnWidth(logicalColumnIndex);
			if (currentColumnWidth <= 0) {
				qDebug() << "Saving column widths: Couldn't read column width for column" << columnName << "in table" << mapper->compTable->name << "- skipping column";
			} else {
				widthsMap[columnName] = currentColumnWidth;
			}
		}
		
		// Column order
		int visualIndex = header->visualIndex(logicalColumnIndex);
		if (visualIndex < 0) {
			qDebug() << "Saving column order: Couldn't read column order for column" << columnName << "in table" << mapper->compTable->name << "- skipping column";
		} else {
			orderMap[columnName] = visualIndex;
		}
	}
	mapper->columnWidthsSetting	->set(*this, widthsMap);
	mapper->columnOrderSetting	->set(*this, orderMap);
	mapper->hiddenColumnsSetting->set(*this, hiddenMap);
}

/**
 * Saves the sorting of the table for the given item type.
 * 
 * @param mapper	The ItemTypeMapper containing the table whose sorting should be saved.
 */
void MainWindow::saveSorting(const ItemTypeMapper* const mapper)
{
	const auto& [column, order] = mapper->compTable->getCurrentSorting();
	QString orderString = order == Qt::DescendingOrder ? "Descending" : "Ascending";
	QString settingValue = column->name + ", " + orderString;
	mapper->sortingSetting->set(*this, settingValue);
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
 * Returns the indices of selected rows and marked row in the table specified by the given
 * ItemTypeMapper.
 * 
 * Marked rows which are not selected are ignored and replaced with the buffer row which corresponds
 * with the lowest view row index of the selected rows.
 * 
 * @return	The selected rows in the currently active table.
 */
QPair<QSet<BufferRowIndex>, BufferRowIndex> MainWindow::getSelectedRows(const ItemTypeMapper& mapper) const
{
	const QModelIndex markedModelIndex = mapper.tableView->currentIndex();
	const QModelIndexList selectedModelIndices = mapper.tableView->selectionModel()->selectedRows();
	
	if (selectedModelIndices.isEmpty()) {
		if (!markedModelIndex.isValid()) return {{}, BufferRowIndex()};
		
		const ViewRowIndex markedViewRow = ViewRowIndex(markedModelIndex.row());
		const BufferRowIndex marked = mapper.compTable->getBufferRowIndexForViewRow(markedViewRow);
		return {{ marked }, marked};
	}
	
	QSet<BufferRowIndex> selected = QSet<BufferRowIndex>();
	for (const QModelIndex& modelIndex : selectedModelIndices) {
		const ViewRowIndex viewRowIndex = ViewRowIndex(modelIndex.row());
		const BufferRowIndex bufferRowIndex = mapper.compTable->getBufferRowIndexForViewRow(viewRowIndex);
		assert(bufferRowIndex.isValid(mapper.compTable->rowCount()));
		selected.insert(bufferRowIndex);
	}
	
	if (markedModelIndex.isValid()) {
		const ViewRowIndex markedViewRow = ViewRowIndex(markedModelIndex.row());
		const BufferRowIndex marked = mapper.compTable->getBufferRowIndexForViewRow(markedViewRow);
		
		if (selected.contains(marked)) {
			return {selected, marked};
		}
	}
	auto bufferRowCompare = [&mapper](const BufferRowIndex& index1, const BufferRowIndex& index2) {
		return mapper.compTable->findViewRowIndexForBufferRow(index1) < mapper.compTable->findViewRowIndexForBufferRow(index2);
	};
	const BufferRowIndex minBufferRow = *std::min_element(selected.constBegin(), selected.constEnd(), bufferRowCompare);
	return {selected, minBufferRow};
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
		windowTitle += "   –   " + filename;
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
