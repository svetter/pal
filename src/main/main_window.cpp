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
 * @file main_window.cpp
 * 
 * This file defines the MainWindow class.
 */

#include "main_window.h"

#include "src/main/about_window.h"
#include "src/main/item_types_handler.h"
#include "src/settings/project_settings_window.h"
#include "src/settings/settings_window.h"
#include "src/tools/relocate_photos_dialog.h"
#include "src/tools/export_dialog.h"
#include "src/viewer/ascent_viewer.h"
#include "ui_main_window.h"

#include <QList>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QCalendarWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>



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
	showDebugTableViews(DEBUG_TABLE_TABS),
	photosDebugTableView(nullptr),
	participatedDebugTableView(nullptr)
{
	setupUi(this);
	createTypesHandler();
	setupMenuIcons();
	statusbar->addPermanentWidget(statusBarTableSizeLabel);
	statusbar->addPermanentWidget(statusBarFiltersLabel);
	itemStatsFrame->setVisible(false);
	setUIEnabled(false);
	
	setWindowTitleFilename();
	
	if (Settings::rememberWindowPositions.get()) {
		setWindowState(Settings::mainWindow_maximized.get() ? Qt::WindowMaximized : Qt::WindowNoState);
		const QRect savedGeometry = Settings::mainWindow_geometry.get();
		if (!savedGeometry.isEmpty()) {
			setGeometry(savedGeometry);
		}
	}
	
	
	ascentFilterBar->supplyPointers(this, &db, (CompositeAscentsTable*) typesHandler->get(ItemTypeAscent)->compTable);
	
	
	connectUI();
	setupTableViews();
	initColumnContextMenu();
	initTableContextMenuAndShortcuts();
	updateRecentFilesMenu();
	
	// Open database
	QString lastOpen = Settings::lastOpenDatabaseFile.get();
	if (!lastOpen.isEmpty() && QFile(lastOpen).exists()) {
		attemptToOpenFile(lastOpen);
	}
	
	
	updateTableContextMenuIcons();
	
	if (showDebugTableViews) setupDebugTableViews();	// After opening database so that auto-sizing columns works correctly
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
	// showFiltersAction is checkable
	autoResizeColumnsAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	resetColumnOrderAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	restoreHiddenColumnsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	// New menu: no fitting icons
	// Tools menu
	relocatePhotosAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	exportDataAction			->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	// Help menu: already has icons
}

/**
 * Creates the ItemTypesHandler singleton.
 * 
 * If debug table views are enabled, also creates the debug tabs and table views.
 */
void MainWindow::createTypesHandler()
{
	QList<QTableView*> debugTableViews = QList<QTableView*>(9, nullptr);
	
	if (showDebugTableViews) {
		QStringList debugTableNames = {
			"ascents",
			"peaks",
			"trips",
			"hikers",
			"regions",
			"ranges",
			"countries",
			"photos",
			"participated"
		};
		
		for (int i = 0; i < debugTableViews.size(); i++) {
			QWidget*		debugTab;
			QHBoxLayout*	debugTabLayout;
			QTableView*		debugTableView;
			
			debugTab = new QWidget();
			debugTab->setObjectName(debugTableNames.at(i) + "DebugTab" + QString::number(i));
			debugTabLayout = new QHBoxLayout(debugTab);
			debugTabLayout->setSpacing(10);
			debugTabLayout->setObjectName(debugTableNames.at(i) + "DebugTabLayout" + QString::number(i));
			debugTabLayout->setContentsMargins(10, 10, 10, 10);
			
			debugTableView = new QTableView(debugTab);
			debugTableView->setObjectName(debugTableNames.at(i) + "DebugTableView" + QString::number(i));
			debugTableView->setContextMenuPolicy(Qt::CustomContextMenu);
			debugTableView->setAlternatingRowColors(true);
			debugTableView->setSelectionMode(QAbstractItemView::SingleSelection);
			debugTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
			debugTableView->setCornerButtonEnabled(false);
			debugTableView->horizontalHeader()->setHighlightSections(false);
			debugTableView->verticalHeader()->setDefaultSectionSize(20);
			debugTableView->verticalHeader()->setHighlightSections(false);
			
			debugTabLayout->addWidget(debugTableView);
			mainAreaTabs->addTab(debugTab, "DBG_" + debugTableNames.at(i));
			
			debugTableViews.replace(i, debugTableView);
		}
	}
	
	typesHandler = new ItemTypesHandler(showDebugTableViews,
		new AscentMapper	(&db, ascentsTab,	ascentsTableView,	debugTableViews.at(0),	ascentsTableAndStatsLayout,	newAscentAction,	newAscentButton,	&db.projectSettings->columnWidths_ascentsTable,		&db.projectSettings->columnOrder_ascentsTable,		&db.projectSettings->hiddenColumns_ascentsTable,	&db.projectSettings->sorting_ascentsTable),
		new PeakMapper		(&db, peaksTab,		peaksTableView,		debugTableViews.at(1),	peaksTabLayout,				newPeakAction,		newPeakButton,		&db.projectSettings->columnWidths_peaksTable,		&db.projectSettings->columnOrder_peaksTable,		&db.projectSettings->hiddenColumns_peaksTable,		&db.projectSettings->sorting_peaksTable),
		new TripMapper		(&db, tripsTab,		tripsTableView,		debugTableViews.at(2),	tripsTabLayout,				newTripAction,		newTripButton,		&db.projectSettings->columnWidths_tripsTable,		&db.projectSettings->columnOrder_tripsTable,		&db.projectSettings->hiddenColumns_tripsTable,		&db.projectSettings->sorting_tripsTable),
		new HikerMapper		(&db, hikersTab,	hikersTableView,	debugTableViews.at(3),	hikersTabLayout,			newHikerAction,		newHikerButton,		&db.projectSettings->columnWidths_hikersTable,		&db.projectSettings->columnOrder_hikersTable,		&db.projectSettings->hiddenColumns_hikersTable,		&db.projectSettings->sorting_hikersTable),
		new RegionMapper	(&db, regionsTab,	regionsTableView,	debugTableViews.at(4),	regionsTabLayout,			newRegionAction,	newRegionButton,	&db.projectSettings->columnWidths_regionsTable,		&db.projectSettings->columnOrder_regionsTable,		&db.projectSettings->hiddenColumns_regionsTable,	&db.projectSettings->sorting_regionsTable),
		new RangeMapper		(&db, rangesTab,	rangesTableView,	debugTableViews.at(5),	rangesTabLayout,			newRangeAction,		newRangeButton,		&db.projectSettings->columnWidths_rangesTable,		&db.projectSettings->columnOrder_rangesTable,		&db.projectSettings->hiddenColumns_rangesTable,		&db.projectSettings->sorting_rangesTable),
		new CountryMapper	(&db, countriesTab,	countriesTableView,	debugTableViews.at(6),	countriesTabLayout,			newCountryAction,	newCountryButton,	&db.projectSettings->columnWidths_countriesTable,	&db.projectSettings->columnOrder_countriesTable,	&db.projectSettings->hiddenColumns_countriesTable,	&db.projectSettings->sorting_countriesTable),
		db.photosTable,
		db.participatedTable
	);
	
	if (showDebugTableViews) {
		photosDebugTableView		= debugTableViews.at(7);
		participatedDebugTableView	= debugTableViews.at(8);
	}
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
	connect(showFiltersAction,				&QAction::changed,				this,	&MainWindow::handle_showFiltersChanged);
	connect(showItemStatsPanelAction,		&QAction::changed,				this,	&MainWindow::handle_showStatsPanelChanged);
	connect(autoResizeColumnsAction,		&QAction::triggered,			this,	&MainWindow::handle_autoResizeColumns);
	connect(resetColumnOrderAction,			&QAction::triggered,			this,	&MainWindow::handle_resetColumnOrder);
	connect(restoreHiddenColumnsAction,		&QAction::triggered,			this,	&MainWindow::handle_restoreHiddenColumns);
	// Menu "New"
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		auto newFunction = [this, mapper] () {
			newItem(mapper);
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
		auto openFunction = [this, mapper] (const QModelIndex& index) {
			if (mapper->type == ItemTypeAscent) {
				viewItem(mapper, ViewRowIndex(index.row()));
			} else {
				editItem(mapper, index);
			}
		};
		connect(mapper->tableView,			&QTableView::doubleClicked,		this,	openFunction);
		
		if (showDebugTableViews) {
			auto editFunctionDebug = [this, mapper] (const QModelIndex& index) {
				mapper->openEditItemDialogAndStoreMethod(this, &db, BufferRowIndex(index.row()));
			};
			connect(mapper->debugTableView,	&QTableView::doubleClicked,		this,	editFunctionDebug);
		}
	}
}

/**
 * Connects each table view to the underlying CompositeTable and to the table context menu.
 */
void MainWindow::setupTableViews()
{
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		// Set model
		mapper->tableView->setModel(mapper->compTable);
		
		// Enable column header reordering
		mapper->tableView->horizontalHeader()->setSectionsMovable(true);
		
		// Enable context menus
		mapper->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(mapper->tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MainWindow::handle_rightClickOnColumnHeader);
		connect(mapper->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClickInTable);
		
		mapper->compTable->setUpdateImmediately(mapper->tableView == getCurrentTableView());
	}
}

/**
 * Connects each debug table view to the underlying Table and to the table context menu.
 */
void MainWindow::setupDebugTableViews()
{
	assert(showDebugTableViews);
	qDebug() << "Showing debugging table tabs in main window";
	
	auto setupFunction = [this] (QTableView* view, Table* table) {
		// Set model
		view->setModel(table);
		view->setRootIndex(table->getNormalRootModelIndex());
		view->resizeColumnsToContents();
		for (int i = 0; i < table->columnCount(); i++) {
			if (view->columnWidth(i) > 400) view->setColumnWidth(i, 400);
		}
		
		// Enable context menu
		connect(view, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClickInTable);
	};
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		setupFunction(mapper->debugTableView, mapper->baseTable);
	}
	
	setupFunction(photosDebugTableView,			db.photosTable);
	setupFunction(participatedDebugTableView,	db.participatedTable);
}

void MainWindow::setupStatisticsTab()
{
	QScatterSeries* peakHeightSeries= new QScatterSeries();
	QScatterSeries* elevGainSeries	= new QScatterSeries();
	QMap<int, int> yearElevGainSums = QMap<int, int>();
	QMap<int, int> yearNumAscents = QMap<int, int>();
	int minYear = INT_MAX;
	int maxYear = INT_MIN;
	for (BufferRowIndex bufferIndex = BufferRowIndex(0); bufferIndex.isValid(db.ascentsTable->getNumberOfRows()); bufferIndex++) {
		Ascent* ascent = db.getAscentAt(bufferIndex);
		
		if (ascent->dateSpecified()) {
			int year = ascent->date.year();
			if (year < minYear) minYear = year;
			if (year > maxYear) maxYear = year;
			
			yearNumAscents[year]++;
			
			qreal dateReal = (qreal) ascent->date.dayOfYear() / ascent->date.daysInYear() + ascent->date.year();
			
			if (ascent->elevationGainSpecified()) {
				int elevGain = ascent->elevationGain;
				elevGainSeries->append(dateReal, elevGain);
				peakHeightSeries->append(dateReal, elevGain);
				yearElevGainSums[year] += elevGain;
			}
			if (ascent->peakID.isValid()) {
				const Peak* const peak = db.getPeak(FORCE_VALID(ascent->peakID));
				if (peak->heightSpecified()) {
					int peakHeight = peak->height;
					peakHeightSeries->append(dateReal, peakHeight);
				}
			}
		}
		
		delete ascent;
	}
	
	QLineSeries* elevGainPerYearSeries		= new QLineSeries();
	QLineSeries* numAscentsPerYearSeries	= new QLineSeries();
	for (int year = minYear; year <= maxYear; year++) {
		int elevGainSum	= yearElevGainSums	.contains(year) ? yearElevGainSums	[year] : 0;
		int numAscents	= yearNumAscents	.contains(year) ? yearNumAscents	[year] : 0;
		elevGainPerYearSeries	->append(year, (qreal) elevGainSum / 1000);
		numAscentsPerYearSeries	->append(year, numAscents);
	}
	elevGainPerYearSeries	->setName("Elevation gain");
	numAscentsPerYearSeries	->setName("Number of ascended peaks");
	
	
	
	QChart* chart1 = new QChart();
	chart1->addSeries(elevGainPerYearSeries);
	chart1->setMargins(QMargins(0, 0, 0, 0));
	chart1->layout()->setContentsMargins(0, 0, 0, 0);
	chart1->setBackgroundRoundness(0);
	chart1->legend()->hide();
	
	QValueAxis* axisX1 = new QValueAxis();
	chart1->addAxis(axisX1, Qt::AlignBottom);
	elevGainPerYearSeries->attachAxis(axisX1);
	axisX1->setLabelFormat("%.0f");
	axisX1->setTickType(QValueAxis::TicksDynamic);
	axisX1->setTickInterval(5);
	axisX1->setTickAnchor(minYear / 5 * 5);
	axisX1->setMinorTickCount(4);
	//axisX2->applyNiceNumbers();
	
	QValueAxis* axisY1 = new QValueAxis();
	chart1->addAxis(axisY1, Qt::AlignLeft);
	elevGainPerYearSeries->attachAxis(axisY1);
	axisY1->setLabelFormat("%.0f");
	axisY1->applyNiceNumbers();
	
	chart1->setTitle("Elevation gain sum per year");
	axisY1->setTitleText("km");
	
	// Create a chart view and set the chart
	QChartView* chartView1 = new QChartView(chart1);
	chartView1->setRenderHint(QPainter::Antialiasing);
	
	
	
	QChart* chart2 = new QChart();
	chart2->addSeries(numAscentsPerYearSeries);
	chart2->setMargins(QMargins(0, 0, 0, 0));
	chart2->layout()->setContentsMargins(0, 0, 0, 0);
	chart2->setBackgroundRoundness(0);
	chart2->legend()->hide();
	
	QValueAxis* axisX2 = new QValueAxis();
	chart2->addAxis(axisX2, Qt::AlignBottom);
	numAscentsPerYearSeries->attachAxis(axisX2);
	axisX2->setLabelFormat("%.0f");
	axisX2->setTickType(QValueAxis::TicksDynamic);
	axisX2->setTickInterval(5);
	axisX2->setTickAnchor(minYear / 5 * 5);
	axisX2->setMinorTickCount(4);
	//axisX2->applyNiceNumbers();
	
	QValueAxis* axisY2 = new QValueAxis();
	chart2->addAxis(axisY2, Qt::AlignLeft);
	numAscentsPerYearSeries->attachAxis(axisY2);
	axisY2->setLabelFormat("%.0f");
	axisY2->applyNiceNumbers();
	
	chart2->setTitle("Number of scaled peaks per year");
	axisY2->setTitleText("# peaks");
	
	// Create a chart view and set the chart
	QChartView* chartView2 = new QChartView(chart2);
	chartView2->setRenderHint(QPainter::Antialiasing);
	
	
	
	QChart* chart3 = new QChart();
	chart3->addSeries(peakHeightSeries);
	chart3->addSeries(elevGainSeries);
	chart3->setMargins(QMargins(0, 0, 0, 0));
	chart3->layout()->setContentsMargins(0, 0, 0, 0);
	chart3->setBackgroundRoundness(0);
	peakHeightSeries->setName("Peak heights");
	elevGainSeries->setName("Elevation gains");
	
	peakHeightSeries->setMarkerShape(QScatterSeries::MarkerShapeTriangle);
	peakHeightSeries->setMarkerSize(6);
	elevGainSeries->setMarkerShape(QScatterSeries::MarkerShapeRotatedRectangle);
	elevGainSeries->setMarkerSize(6);
	QPen pen(Qt::transparent);
	peakHeightSeries->setPen(pen);
	elevGainSeries->setPen(pen);
	
	QValueAxis* axisX3 = new QValueAxis();
	chart3->addAxis(axisX3, Qt::AlignBottom);
	peakHeightSeries->attachAxis(axisX3);
	elevGainSeries->attachAxis(axisX3);
	axisX3->setLabelFormat("%.0f");
	axisX3->setTickType(QValueAxis::TicksDynamic);
	axisX3->setTickInterval(2);
	axisX3->setTickAnchor(minYear / 2 * 2);
	axisX3->setMinorTickCount(1);
	
	QValueAxis* axisY3 = new QValueAxis();
	chart3->addAxis(axisY3, Qt::AlignLeft);
	peakHeightSeries->attachAxis(axisY3);
	elevGainSeries->attachAxis(axisY3);
	axisY3->setLabelFormat("%.0f");
	axisY3->applyNiceNumbers();
	
	chart3->setTitle("All elevation gains and peak heights over time");
	axisY3->setTitleText("m");
	
	// Create a chart view and set the chart
	QChartView* chartView3 = new QChartView(chart3);
	chartView3->setRenderHint(QPainter::Antialiasing);
	
	
	
	statisticsTabUpperLayout->addWidget(chartView1);
	statisticsTabUpperLayout->addWidget(chartView2);
	statisticsTabUpperLayout->setStretch(0, 1);
	statisticsTabUpperLayout->setStretch(1, 1);
	statisticsTabLayout->addWidget(chartView3);
	statisticsTabLayout->setStretch(0, 2);
	statisticsTabLayout->setStretch(1, 3);
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
		const QString& columnName = mapper->compTable->getColumnAt(columnIndex)->name;
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
	QList<QPair<QString, int>> columnOrderList = QList<QPair<QString, int>>();
	for (const QPair<QString, int>& columnOrderPair : columnOrderMap.asKeyValueRange()) {
		if (columnOrderPair.second < 0) continue;	// Visual index invalid, ignore column
		columnOrderList.append(columnOrderPair);
	}
	auto comparator = [](const QPair<QString, int>& pair1, const QPair<QString, int>& pair2) {
		return pair1.second < pair2.second;
	};
	std::sort(columnOrderList.begin(), columnOrderList.end(), comparator);
	
	// Restore column order
	QHeaderView* header = mapper->tableView->horizontalHeader();
	for (int visualIndex = 0; visualIndex < columnOrderList.size(); visualIndex++) {
		const QString& columnName = columnOrderList.at(visualIndex).first;
		int logicalIndex = mapper->compTable->getColumnByName(columnName)->getIndex();
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
		const QString& columnName = mapper->compTable->getColumnAt(columnIndex)->name;
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
		
		const CompositeColumn* column = mapper->compTable->getColumnByName(saved.at(0).trimmed());
		if (!column) break;
		
		bool ascending = saved.at(1).trimmed().compare("Descending", Qt::CaseInsensitive) != 0;
		Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
		
		sorting.column = column;
		sorting.order = order;
		sortingSettingValid = true;
		break;
	}
	mapper->tableView->sortByColumn(sorting.column->getIndex(), sorting.order);
	
	if (!sortingSettingValid) mapper->sortingSetting->clear(this);
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
	
	connect(openAction,			&QAction::triggered,	this,	&MainWindow::handle_viewSelectedItem);
	connect(editAction,			&QAction::triggered,	this,	&MainWindow::handle_editSelectedItem);
	connect(duplicateAction,	&QAction::triggered,	this,	&MainWindow::handle_duplicateAndEditSelectedItem);
	connect(deleteAction,		&QAction::triggered,	this,	&MainWindow::handle_deleteSelectedItems);
	
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
		
		connect(openShortcut,		&QShortcut::activated, this, &MainWindow::handle_viewSelectedItem);
		connect(editShortcut,		&QShortcut::activated, this, &MainWindow::handle_editSelectedItem);
		connect(duplicateShortcut,	&QShortcut::activated, this, &MainWindow::handle_duplicateAndEditSelectedItem);
		connect(deleteShortcut,		&QShortcut::activated, this, &MainWindow::handle_deleteSelectedItems);
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
	QIcon icon = QIcon(":/icons/" + getActiveMapper()->name + ".svg");
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
	bool dbOpened = db.openExisting(this, filepath);
	
	if (dbOpened) {
		setWindowTitleFilename(filepath);
		updateFilters();
		
		// Restore project-specific implicit settings:
		// Filter bar
		showFiltersAction->setChecked(db.projectSettings->mainWindow_showFilterBar.get(this));
		// Item statistics panel
		showItemStatsPanelAction->setChecked(db.projectSettings->mainWindow_showItemStatsPanel.get(this));
		// Open tab
		if (Settings::rememberTab.get()) {
			mainAreaTabs->setCurrentIndex(db.projectSettings->mainWindow_currentTabIndex.get(this));
		}
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
		updateTableSize();
		
		handle_showFiltersChanged();
		handle_showStatsPanelChanged();
		
		setUIEnabled(true);
		addToRecentFilesList(filepath);
		
		setupStatisticsTab();
	}
	projectOpen = dbOpened;
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
	QTableView* currentTableView = getCurrentTableView();
	
	int numCells = 0;
	if (prepareAll) {
		for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
			numCells += mapper->compTable->getNumberOfCellsToInit();
		}
	} else {
		numCells += getActiveMapper()->compTable->getNumberOfCellsToInit();
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
	typesHandler->get(ItemTypeAscent)->compTable->setInitialFilters(ascentFilters);
	
	for (ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		progress.setLabelText(tr("Preparing table %1...").arg(mapper->baseTable->uiName));
		
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
	saveDatabaseAsAction		->setEnabled(enabled);
	closeDatabaseAction			->setEnabled(enabled);
	projectSettingsAction		->setEnabled(enabled);
	viewMenu					->setEnabled(enabled);
	newMenu						->setEnabled(enabled);
	toolsMenu					->setEnabled(enabled);
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		mapper->newItemButton->setEnabled(enabled);
	}
	mainAreaTabs				->setEnabled(enabled);
	ascentCounterSegmentNumber	->setEnabled(enabled);
	ascentCounterLabel			->setEnabled(enabled);
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
 * table.
 * 
 * For the ascent table, also updates the status bar indicator for the number of applied filters.
 * 
 * @param reset	Clear the status bar indicator completely.
 */
void MainWindow::updateTableSize(bool reset)
{
	if (reset) {
		statusBarTableSizeLabel->setText("");
		statusBarFiltersLabel->setText("");
		ascentCounterSegmentNumber->setProperty("value", QVariant());
		return;
	}
	
	bool debugTable;
	const ItemTypeMapper* const mapper = typesHandler->getMatchingMapper(getCurrentTableView(), &debugTable);
	
	QString countText = QString();
	int total = mapper->baseTable->getNumberOfRows();
	if (total == 0) {
		countText = tr("Table is empty");
	}
	else if (mapper->type == ItemTypeAscent && !debugTable) {
		int displayed = mapper->compTable->rowCount();
		int filtered = total - displayed;
		countText = (total == 1 ? tr("%2 of %1 entry shown (%3 filtered out)") : tr("%2 of %1 entries shown (%3 filtered out)")).arg(total).arg(displayed).arg(filtered);
	} else {
		countText = (total == 1 ? tr("%1 entry") : tr("%1 entries")).arg(total);
	}
	statusBarTableSizeLabel->setText(countText);
	
	QString filterText = QString();
	if (mapper->type == ItemTypeAscent && !debugTable) {
		int filtersApplied = mapper->compTable->getCurrentFilters().size();
		if (filtersApplied) {
			filterText = (filtersApplied == 1 ? tr("%1 filter applied") : tr("%1 filters applied")).arg(filtersApplied);
		} else {
			filterText = tr("No filters applied");
		}
	}
	statusBarFiltersLabel->setText(filterText);
	
	ascentCounterSegmentNumber->setProperty("value", QVariant(db.ascentsTable->getNumberOfRows()));
}



// EXECUTE USER COMMANDS

/**
 * Opens the item specified by the given ItemTypeMapper and view row index for viewing.
 * 
 * @param mapper		The ItemTypeMapper for the type of item to open.
 * @param viewRowIndex	The view row index of the item to open.
 */
void MainWindow::viewItem(const ItemTypeMapper* const mapper, ViewRowIndex viewRowIndex)
{
	switch (mapper->type) {
	case ItemTypeAscent:
		AscentViewer(this, &db, typesHandler, viewRowIndex).exec();
		return;
	default:
		assert(false);
	}
}

/**
 * Opens a dialog for creating a new item of the type specified by the given ItemTypeMapper.
 * 
 * If a new item was created, performs the necessary updates to the UI.
 * 
 * @param mapper	The ItemTypeMapper for the type of item to create.
 */
void MainWindow::newItem(const ItemTypeMapper* const mapper)
{
	BufferRowIndex newBufferRowIndex = mapper->openNewItemDialogAndStoreMethod(this, &db);
	if (newBufferRowIndex == -1) return;
	
	performUpdatesAfterUserAction(mapper, true, newBufferRowIndex);
	setStatusLine(tr("Saved new %1.").arg(mapper->baseTable->getItemNameSingularLowercase()));
}

/**
 * Opens a dialog for creating a new item of the type specified by the given ItemTypeMapper as a
 * duplicate of the item specified by the given view row index.
 * 
 * If a new item was created, performs the necessary updates to the UI.
 * 
 * @param mapper		The ItemTypeMapper for the type of item to duplicate.
 * @param viewRowIndex	The view row index of the item to duplicate.
 */
void MainWindow::duplicateAndEditItem(const ItemTypeMapper* const mapper, ViewRowIndex viewRowIndex)
{
	BufferRowIndex bufferRowIndex = mapper->compTable->getBufferRowIndexForViewRow(viewRowIndex);
	BufferRowIndex newBufferRowIndex = mapper->openDuplicateItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	if (newBufferRowIndex == -1) return;
	
	performUpdatesAfterUserAction(mapper, true, newBufferRowIndex);
	setStatusLine(tr("Saved new %1.").arg(mapper->baseTable->getItemNameSingularLowercase()));
}

/**
 * Opens a dialog for editing the item specified by the given ItemTypeMapper and view row index.
 * 
 * If the item was edited, performs the necessary updates to the UI.
 * 
 * @param mapper		The ItemTypeMapper for the type of item to edit.
 * @param viewRowIndex	The view row index of the item to edit.
 */
void MainWindow::editItem(const ItemTypeMapper* const mapper, const QModelIndex& index)
{
	ViewRowIndex viewRowIndex = ViewRowIndex(index.row());
	BufferRowIndex bufferRowIndex = mapper->compTable->getBufferRowIndexForViewRow(viewRowIndex);
	mapper->openEditItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	
	performUpdatesAfterUserAction(mapper, false, bufferRowIndex);
	setStatusLine(tr("Saved changes in %1.").arg(mapper->baseTable->getItemNameSingularLowercase()));
}

/**
 * Opens a dialog for deleting the items specified by the given ItemTypeMapper and a set of view row
 * indices.
 * 
 * If the item was deleted, performs the necessary updates to the UI.
 * 
 * @param mapper			The ItemTypeMapper for the type of item to delete.
 * @param viewRowIndices	The view row indices of the items to delete.
 */
void MainWindow::deleteItems(const ItemTypeMapper* const mapper, QSet<ViewRowIndex> viewRowIndices)
{
	if (viewRowIndices.isEmpty()) return;
	
	QSet<BufferRowIndex> bufferRowIndices = QSet<BufferRowIndex>();
	for (const ViewRowIndex& viewRowIndex : viewRowIndices) {
		bufferRowIndices += mapper->compTable->getBufferRowIndexForViewRow(viewRowIndex);
	}
	
	mapper->openDeleteItemsDialogAndExecuteMethod(this, &db, bufferRowIndices);
	
	performUpdatesAfterUserAction(mapper, true);
	setStatusLine(tr("Deleted %1.").arg(mapper->baseTable->getItemNameSingularLowercase()));
}


/**
 * Updates selection, table size info and filters after an item was added, edited or removed.
 * 
 * @param mapper					The ItemTypeMapper for the table that was changed.
 * @param numberOfEntriesChanged	Whether the number of entries in the table changed.
 * @param bufferRowToSelectIndex	The buffer row index of the item to select after the update.
 */
void MainWindow::performUpdatesAfterUserAction(const ItemTypeMapper* const mapper, bool numberOfEntriesChanged, BufferRowIndex bufferRowToSelectIndex)
{
	// Update selection in table
	if (bufferRowToSelectIndex.isValid()) {
		ViewRowIndex viewRowToSelectIndex = mapper->compTable->findViewRowIndexForBufferRow(bufferRowToSelectIndex);
		updateSelectionAfterUserAction(mapper, viewRowToSelectIndex);
	}
	// Update table size info
	if (numberOfEntriesChanged)	updateTableSize();
	// Update filters
	updateFilters();
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
void MainWindow::updateSelectionAfterUserAction(const ItemTypeMapper* const mapper, ViewRowIndex viewRowIndex)
{
	QModelIndex modelIndex = mapper->compTable->index(viewRowIndex.get(), 0);
	mapper->tableView->setCurrentIndex(modelIndex);
	mapper->tableView->scrollTo(modelIndex);
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
	const ItemTypeMapper* const mapperForOpenTab = getActiveMapper();
	mapperForOpenTab->tableAndStatsLayout->addWidget(itemStatsFrame);
	mapperForOpenTab->tableAndStatsLayout->setStretch(0, 3);
	mapperForOpenTab->tableAndStatsLayout->setStretch(1, 1);
	
	if (!projectOpen) return;
	
	QProgressDialog progress(this);
	progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowCloseButtonHint);
	progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Updating table"));
	progress.setLabel(new QLabel(tr("Updating table..."), &progress));
	progress.setMinimumWidth(250);
	progress.setCancelButton(nullptr);
	progress.setMinimumDuration(500);
	
	for (ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		if (mapper == mapperForOpenTab) {
			progress.setMaximum(mapper->compTable->getNumberOfCellsToUpdate());
			
			mapper->compTable->setUpdateImmediately(true, &progress);
			mapper->openingTab();
		}
		else {
			mapper->compTable->setUpdateImmediately(false);
		}
	}
	
	updateTableSize();
	updateTableContextMenuIcons();
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
	const ItemTypeMapper* const mapper = getActiveMapper();
	
	// Get index of clicked column
	QHeaderView* header = mapper->tableView->horizontalHeader();
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
		
		const QString& columnName = mapper->compTable->getColumnAt(logicalIndex)->uiName;
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
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex index = currentTableView->indexAt(pos);
	if (!index.isValid()) return;
	
	bool singleRowSelected = currentTableView->selectionModel()->selectedRows().size() == 1;
	bool viewableItemTable = currentTableView == ascentsTableView;
	bool duplicatableItemTable = currentTableView == ascentsTableView || currentTableView == peaksTableView;
	
	tableContextMenuOpenAction		->setVisible(singleRowSelected && viewableItemTable);
	tableContextMenuEditAction		->setVisible(singleRowSelected);
	tableContextMenuDuplicateAction	->setVisible(singleRowSelected && duplicatableItemTable);
	
	QString deleteString = tr("Delete") + (Settings::confirmDelete.get() ? "..." : "");
	tableContextMenuDeleteAction->setText(deleteString);
	
	tableContextMenu.popup(currentTableView->viewport()->mapToGlobal(pos));
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
	
	const ItemTypeMapper* const mapper = getActiveMapper();
	mapper->tableView->horizontalHeader()->setSectionHidden(logicalIndex, true);
	mapper->compTable->markColumnHidden(logicalIndex);
	
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
	
	const ItemTypeMapper* const mapper = getActiveMapper();
	mapper->tableView->horizontalHeader()->setSectionHidden(logicalIndex, false);
	mapper->compTable->markColumnUnhidden(logicalIndex);
	mapper->compTable->updateBothBuffers();
}



// TABLE CONTEXT MENU ACTION HANDLERS

/**
 * Event handler for the view action in the table context menu.
 * 
 * Opens the currently selected item in the active table for viewing.
 */
void MainWindow::handle_viewSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	bool debugTable;
	const ItemTypeMapper* const mapper = typesHandler->getMatchingMapper(getCurrentTableView(), &debugTable);
	
	if (debugTable) {
		mapper->openEditItemDialogAndStoreMethod(this, &db, BufferRowIndex(selectedIndex.row()));
	} else if (mapper->type == ItemTypeAscent) {
		viewItem(mapper, ViewRowIndex(selectedIndex.row()));
	} else {
		editItem(mapper, selectedIndex);
	}
}

/**
 * Event handler for the edit action in the table context menu.
 * 
 * Opens the currently selected item in the active table for editing.
 */
void MainWindow::handle_editSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	bool debugTable;
	const ItemTypeMapper* const mapper = typesHandler->getMatchingMapper(getCurrentTableView(), &debugTable);
	
	if (debugTable) {
		mapper->openEditItemDialogAndStoreMethod(this, &db, BufferRowIndex(selectedIndex.row()));
	} else {
		editItem(mapper, selectedIndex);
	}
}

/**
 * Event handler for the duplicate action in the table context menu.
 * 
 * Opens the currently selected item in the active table for editing as a new duplicate.
 */
void MainWindow::handle_duplicateAndEditSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	bool debugTable;
	const ItemTypeMapper* const mapper = typesHandler->getMatchingMapper(getCurrentTableView(), &debugTable);
	
	if (debugTable) {
		mapper->openDuplicateItemDialogAndStoreMethod(this, &db, BufferRowIndex(selectedIndex.row()));
	} else {
		duplicateAndEditItem(mapper, ViewRowIndex(selectedIndex.row()));
	}
}

/**
 * Event handler for the delete action in the table context menu.
 * 
 * Opens a dialog for deleting the currently selected items in the active table.
 */
void MainWindow::handle_deleteSelectedItems()
{
	QTableView* currentTableView = getCurrentTableView();
	QSet<ViewRowIndex> selectedViewRowIndices = QSet<ViewRowIndex>();
	for (const QModelIndex& index : currentTableView->selectionModel()->selectedRows()) {
		selectedViewRowIndices += ViewRowIndex(index.row());
	}
	if (selectedViewRowIndices.isEmpty()) return;
	
	bool debugTable;
	const ItemTypeMapper* const mapper = typesHandler->getMatchingMapper(getCurrentTableView(), &debugTable);
	
	if (debugTable) {
		QSet<BufferRowIndex> selectedBufferRowIndices = QSet<BufferRowIndex>();
		for (const ViewRowIndex& viewRowIndex : qAsConst(selectedViewRowIndices)) {
			selectedBufferRowIndices += BufferRowIndex(viewRowIndex.get());
		}
		mapper->openDeleteItemsDialogAndExecuteMethod(this, &db, selectedBufferRowIndices);
	} else {
		deleteItems(mapper, selectedViewRowIndices);
	}
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
	db.createNew(this, filepath);
	updateFilters();
	updateTableSize();
	setUIEnabled(true);
	
	addToRecentFilesList(filepath);
	
	if (Settings::openProjectSettingsOnNewDatabase.get()) {
		ProjectSettingsWindow(this, &db, true).exec();
	}
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
	
	bool success = db.saveAs(this, filepath);
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
	}
	updateTableSize(true);
}


/**
 * Event handler for the "project settings" action in the file menu.
 * 
 * Opens the project settings dialog.
 */
void MainWindow::handle_openProjectSettings()
{
	ProjectSettingsWindow(this, &db).exec();
}

/**
 * Event handler for the "settings" action in the file menu.
 * 
 * Opens the settings dialog.
 */
void MainWindow::handle_openSettings()
{
	SettingsWindow(this).exec();
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
	if (!showFilters) ascentFilterBar->handle_clearFilters();
}
	
/**
 * Event handler for the "show statistics panel" action in the view menu.
 * 
 * Shows or hides the item-related statistics panel.
 */
void MainWindow::handle_showStatsPanelChanged()
{
	bool showStatsPanel = showItemStatsPanelAction->isChecked();
	itemStatsFrame->setVisible(showStatsPanel);
}

/**
 * Event handler for the "auto-resize columns" action in the view menu.
 * 
 * Resizes all columns in the currently active table to fit their contents.
 */
void MainWindow::handle_autoResizeColumns()
{
	bool debugTable;
	const ItemTypeMapper* const mapper = typesHandler->getMatchingMapper(getCurrentTableView(), &debugTable);
	
	QTableView* tableView = debugTable ? mapper->debugTableView : mapper->tableView;
	int numColumns = debugTable ? mapper->baseTable->getNumberOfColumns() : mapper->compTable->columnCount();
	
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
	QHeaderView* header = getActiveMapper()->tableView->horizontalHeader();
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
	const ItemTypeMapper* const mapper = getActiveMapper();
	QHeaderView* const header = mapper->tableView->horizontalHeader();
	for (int columnIndex = 0; columnIndex < header->count(); columnIndex++) {
		header->setSectionHidden(columnIndex, false);
	}
	mapper->compTable->markAllColumnsUnhidden();
	mapper->compTable->updateBothBuffers();
}



// TOOLS MENU ACTION HANDLERS

/**
 * Event handler for the "relocate photos" action in the tools menu.
 * 
 * Opens the photo relocation dialog.
 */
void MainWindow::handle_relocatePhotos()
{
	RelocatePhotosDialog(this, &db).exec();
}

/**
 * Event handler for the "export data" action in the tools menu.
 * 
 * Opens the photo relocation dialog.
 */
void MainWindow::handle_exportData()
{
	DataExportDialog(this, typesHandler).exec();
}



// HELP MENU ACTION HANDLERS

/**
 * Event handler for the "about PAL" action in the help menu.
 * 
 * Opens the about dialog.
 */
void MainWindow::handle_about()
{
	AboutWindow(this).exec();
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
	
	db.projectSettings->mainWindow_currentTabIndex		.set(this, mainAreaTabs->currentIndex());
	db.projectSettings->mainWindow_showFilterBar		.set(this, showFiltersAction->isChecked());
	db.projectSettings->mainWindow_showItemStatsPanel	.set(this, showItemStatsPanelAction->isChecked());
	
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
}

/**
 * Saves all column-related implicit project settings for the given item type.
 * 
 * @param mapper	The ItemTypeMapper containing the table whose column widths should be saved.
 */
void MainWindow::saveImplicitColumnSettings(const ItemTypeMapper* const mapper)
{
	if (!mapper->tabHasBeenOpened()) return;	// Only save if table was actually shown
	QHeaderView* header = mapper->tableView->horizontalHeader();
	
	QMap<QString, int>	widthsMap;
	QMap<QString, int>	orderMap;
	QMap<QString, bool>	hiddenMap;
	for (int logicalColumnIndex = 0; logicalColumnIndex < mapper->compTable->columnCount(); logicalColumnIndex++) {
		const QString& columnName = mapper->compTable->getColumnAt(logicalColumnIndex)->name;
		
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
	mapper->columnWidthsSetting	->set(this, widthsMap);
	mapper->columnOrderSetting	->set(this, orderMap);
	mapper->hiddenColumnsSetting->set(this, hiddenMap);
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
	mapper->sortingSetting->set(this, settingValue);
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
	Q_UNUSED(event);
	if (!isVisible()) {		// Window is still being initialized
		event->ignore();
		return;
	}
	
	for (const ItemTypeMapper* const mapper : typesHandler->getAllMappers()) {
		QPushButton* button = mapper->newItemButton;
		bool buttonVisible = button->x() + button->width() < ascentCounterLabel->x() - 40;
		button->setVisible(buttonVisible);
	}
	event->accept();
}





// GENERAL HELPERS

/**
 * Returns the view of the table in the currently active tab.
 * 
 * @return	The active table view.
 */
QTableView* MainWindow::getCurrentTableView() const
{
	return mainAreaTabs->currentWidget()->findChild<QTableView*>();
}

/**
 * Returns the ItemTypeMapper for the currently active tab.
 * 
 * @return	The active ItemTypeMapper.
 */
const ItemTypeMapper* MainWindow::getActiveMapper() const
{
	return typesHandler->getMatchingMapper(getCurrentTableView());
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
