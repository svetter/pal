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

#include "main_window.h"

#include "src/main/about_window.h"
#include "src/main/item_types_handler.h"
#include "src/main/project_settings_window.h"
#include "src/main/settings_window.h"
#include "src/tools/relocate_photos_dialog.h"
#include "src/viewer/ascent_viewer.h"
#include "ui_main_window.h"

#include <QList>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QCalendarWidget>



MainWindow::MainWindow() :
		QMainWindow(nullptr),
		Ui_MainWindow(),
		db(Database()),
		openRecentActions(QList<QAction*>()),
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
	
	if (Settings::rememberTab.get()) {
		mainAreaTabs->setCurrentIndex(Settings::mainWindow_currentTabIndex.get());
	}
	
	showFiltersAction->setChecked(Settings::mainWindow_showFilters.get());
	
	
	createTypesHandler();
	
	
	ascentFilterBar->supplyPointers(this, &db, (CompositeAscentsTable*) typesHandler->get(Ascent)->compTable);
	
	
	connectUI();
	setupTableViews();
	initTableContextMenuAndShortcuts();
	updateRecentFilesMenu();
	
	handle_showFiltersChanged();
	
	// Open database
	QString lastOpen = Settings::lastOpenDatabaseFile.get();
	if (!lastOpen.isEmpty() && QFile(lastOpen).exists()) {
		setWindowTitleFilename(lastOpen);
		db.openExisting(this, lastOpen);
		updateFilters();
		setVisible(true);
		initCompositeBuffers();
		updateTableSize();
		setUIEnabled(true);
	}
	
	
	if (showDebugTableViews) setupDebugTableViews();	// After opening database so that auto-sizing columns works correctly
}

MainWindow::~MainWindow()
{
	qDeleteAll(shortcuts);
	delete typesHandler;
}



// INITIAL SETUP

void MainWindow::setupMenuIcons()
{
	newDatabaseAction		->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
	openDatabaseAction		->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	openRecentMenu			->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
	saveDatabaseAsAction	->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	closeDatabaseAction		->setIcon(style()->standardIcon(QStyle::SP_TabCloseButton));
	projectSettingsAction	->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	settingsAction			->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	// View menu
	// showFiltersAction is checkable
	autoResizeColumnsAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	// New menu: no fitting icons
	// Tools menu
	relocatePhotosAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	// Help menu: already has icons
}

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
		new AscentTypeMapper	(&db, ascentsTab,	ascentsTableView,	debugTableViews.at(0),	newAscentAction,	newAscentButton),
		new PeakTypeMapper		(&db, peaksTab,		peaksTableView,		debugTableViews.at(1),	newPeakAction,		newPeakButton),
		new TripTypeMapper		(&db, tripsTab,		tripsTableView,		debugTableViews.at(2),	newTripAction,		newTripButton),
		new HikerTypeMapper		(&db, hikersTab,	hikersTableView,	debugTableViews.at(3),	newHikerAction,		nullptr),
		new RegionTypeMapper	(&db, regionsTab,	regionsTableView,	debugTableViews.at(4),	newRegionAction,	nullptr),
		new RangeTypeMapper		(&db, rangesTab,	rangesTableView,	debugTableViews.at(5),	newRangeAction,		nullptr),
		new CountryTypeMapper	(&db, countriesTab,	countriesTableView,	debugTableViews.at(6),	newCountryAction,	nullptr)
	);
	
	if (showDebugTableViews) {
		photosDebugTableView		= debugTableViews.at(7);
		participatedDebugTableView	= debugTableViews.at(8);
	}
}

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
	connect(autoResizeColumnsAction,		&QAction::triggered,			this,	&MainWindow::handle_autoResizeColumns);
	// Menu "New"
	typesHandler->forEach([this] (const ItemTypeMapper& mapper) {
		auto newFunction = [this, &mapper] () {
			newItem(mapper);
		};
		
		connect(mapper.newItemAction,		&QAction::triggered,			this,	newFunction);
		// Big new item buttons
		if (mapper.newItemButton) {
			connect(mapper.newItemButton,	&QPushButton::clicked,			this,	newFunction);
		}
	});
	// Menu "Tools"
	connect(relocatePhotosAction,			&QAction::triggered,			this,	&MainWindow::handle_relocatePhotos);
	// Menu "Help"
	connect(aboutAction,					&QAction::triggered,			this,	&MainWindow::handle_about);
	connect(aboutQtAction,					&QAction::triggered,			this,	&QApplication::aboutQt);
	
	// === TABS AND TABLES ===
	
	// Tabs
	connect(mainAreaTabs,					&QTabWidget::currentChanged,	this,	&MainWindow::handle_tabChanged);
	// Double clicks on table
	typesHandler->forEach([this] (const ItemTypeMapper& mapper) {
		auto openFunction = [this, &mapper] (const QModelIndex& index) {
			if (mapper.type == Ascent) {
				viewItem(mapper, index.row());
			} else {
				editItem(mapper, index);
			}
		};
		connect(mapper.tableView,			&QTableView::doubleClicked,		this,	openFunction);
		
		if (showDebugTableViews) {
			auto editFunctionDebug = [this, &mapper] (const QModelIndex& index) {
				mapper.openEditItemDialogAndStoreMethod(this, &db, index.row());
			};
			connect(mapper.debugTableView,	&QTableView::doubleClicked,		this,	editFunctionDebug);
		}
	});
}

void MainWindow::setupTableViews()
{
	typesHandler->forEach([this] (const ItemTypeMapper& mapper) {
		// Set model
		mapper.tableView->setModel(mapper.compTable);
		
		if (Settings::rememberColumnWidths.get()) {
			restoreColumnWidths(mapper);
		}
		
		// Enable context menu
		connect(mapper.tableView, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClick);
		
		mapper.compTable->setUpdateImmediately(mapper.tableView == getCurrentTableView());
		
		setSorting(mapper);
	});
}

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
		connect(view, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClick);
	};
	
	typesHandler->forEach([&setupFunction] (const ItemTypeMapper& mapper) {
		setupFunction(mapper.debugTableView, mapper.baseTable);
	});
	
	setupFunction(photosDebugTableView,			db.photosTable);
	setupFunction(participatedDebugTableView,	db.participatedTable);
}

void MainWindow::restoreColumnWidths(const ItemTypeMapper& mapper)
{
	QStringList columnWidths = mapper.columnWidthsSetting->get();
	if (columnWidths.size() != mapper.compTable->columnCount()) {
		// Can't restore column widths from settings
		if (!columnWidths.isEmpty())
			qDebug() << QString("Couldn't restore column widths for table %1: Expected %2 numbers, but got %3")
					.arg(mapper.compTable->name).arg(mapper.compTable->columnCount()).arg(columnWidths.size());
		
		mapper.columnWidthsSetting->clear();
		return mapper.tableView->resizeColumnsToContents();
	}
	
	// Restore column widths
	for (int i = 0; i < mapper.compTable->columnCount(); i++) {
		mapper.tableView->setColumnWidth(i, columnWidths.at(i).toInt());
	}
}

void MainWindow::setSorting(const ItemTypeMapper& mapper)
{
	QPair<const CompositeColumn*, Qt::SortOrder> sorting = mapper.compTable->getDefaultSorting();
	bool sortingSettingValid = true;
	
	while (Settings::rememberSorting.get() && mapper.sortingSetting->present()) {
		sortingSettingValid = false;
		
		QStringList saved = mapper.sortingSetting->get();
		if (saved.size() != 2) break;
		bool canConvert = false;
		saved.at(0).toInt(&canConvert);
		if (!canConvert) break;
		
		int columnIndex = saved.at(0).toInt();
		bool ascending = saved.at(1).compare("Descending", Qt::CaseInsensitive) != 0;
		Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
		sorting.first = mapper.compTable->getColumnAt(columnIndex);
		sorting.second = order;
		
		sortingSettingValid = true;
		break;
	}
	mapper.tableView->sortByColumn(sorting.first->getIndex(), sorting.second);
	
	if (!sortingSettingValid) mapper.sortingSetting->clear();
}


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
	QAction* deleteAction		= tableContextMenu.addAction(tr("Delete..."),					deleteKeySequence);
	// store actions for open and duplicate (for disbling them where they're not needed)
	tableContextMenuOpenAction		= openAction;
	tableContextMenuDuplicateAction	= duplicateAction;
	
	connect(openAction,			&QAction::triggered,	this,	&MainWindow::handle_viewSelectedItem);
	connect(editAction,			&QAction::triggered,	this,	&MainWindow::handle_editSelectedItem);
	connect(duplicateAction,	&QAction::triggered,	this,	&MainWindow::handle_duplicateAndEditSelectedItem);
	connect(deleteAction,		&QAction::triggered,	this,	&MainWindow::handle_deleteSelectedItem);
	
	// Keyboard shortcuts
	QList<QTableView*> tableViews = mainAreaTabs->findChildren<QTableView*>();
	for (auto iter = tableViews.constBegin(); iter != tableViews.constEnd(); iter++) {
		QShortcut* openShortcut			= new QShortcut(openKeySequence,		*iter);
		QShortcut* editShortcut			= new QShortcut(editKeySequence,		*iter);
		QShortcut* duplicateShortcut	= new QShortcut(duplicateKeySequence,	*iter);
		QShortcut* deleteShortcut		= new QShortcut(deleteKeySequence,		*iter);
		
		shortcuts.append(openShortcut);
		shortcuts.append(editShortcut);
		shortcuts.append(duplicateShortcut);
		shortcuts.append(deleteShortcut);
		
		connect(openShortcut,		&QShortcut::activated, this, &MainWindow::handle_viewSelectedItem);
		connect(editShortcut,		&QShortcut::activated, this, &MainWindow::handle_editSelectedItem);
		connect(duplicateShortcut,	&QShortcut::activated, this, &MainWindow::handle_duplicateAndEditSelectedItem);
		connect(deleteShortcut,		&QShortcut::activated, this, &MainWindow::handle_deleteSelectedItem);
	}
}



// PROJECT SETUP

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
		typesHandler->forEach([&numCells] (const ItemTypeMapper& mapper) {
			numCells += mapper.compTable->getNumberOfCellsToInit();
		});
	} else {
		typesHandler->forMatchingTableView(currentTableView, [&numCells] (const ItemTypeMapper& mapper, bool debug) {
			Q_UNUSED(debug);
			numCells += mapper.compTable->getNumberOfCellsToInit();
		});
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
	typesHandler->get(Ascent)->compTable->setInitialFilters(ascentFilters);
	
	typesHandler->forEach([&progress, prepareAll, currentTableView] (const ItemTypeMapper& mapper) {
		progress.setLabelText(tr("Preparing table %1...").arg(mapper.baseTable->uiName));
		
		bool prepareThisTable = prepareAll || mapper.tableView == currentTableView;
		bool autoResizeColumns = !Settings::rememberColumnWidths.get() || !mapper.columnWidthsSetting->present();
		QProgressDialog* updateProgress = prepareThisTable ? &progress : nullptr;
		bool deferCompute = !prepareThisTable;
		QTableView* tableToAutoResizeAfterCompute = autoResizeColumns ? mapper.tableView : nullptr;
		
		mapper.compTable->initBuffer(updateProgress, deferCompute, tableToAutoResizeAfterCompute);
	});
}



// UI UPDATES

void MainWindow::setUIEnabled(bool enabled)
{
	mainAreaTabs				->setEnabled(enabled);
	newAscentButton				->setEnabled(enabled);
	newPeakButton				->setEnabled(enabled);
	newTripButton				->setEnabled(enabled);
	ascentCounterSegmentNumber	->setEnabled(enabled);
	ascentCounterLabel			->setEnabled(enabled);
	
	saveDatabaseAsAction		->setEnabled(enabled);
	closeDatabaseAction			->setEnabled(enabled);
	projectSettingsAction		->setEnabled(enabled);
	
	viewMenu					->setEnabled(enabled);
	newMenu						->setEnabled(enabled);
	toolsMenu					->setEnabled(enabled);
}

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

void MainWindow::updateTableSize(bool reset)
{
	if (reset) {
		statusBarTableSizeLabel->setText("");
		statusBarFiltersLabel->setText("");
		ascentCounterSegmentNumber->setProperty("value", QVariant());
		return;
	}
	
	typesHandler->forMatchingTableView(getCurrentTableView(), [this] (const ItemTypeMapper& mapper, bool debugTable) {
		QString countText = QString();
		int total = mapper.baseTable->getNumberOfRows();
		if (total == 0) {
			countText = tr("Table is empty");
		}
		else if (mapper.type == Ascent && !debugTable) {
			int displayed = mapper.compTable->rowCount();
			int filtered = total - displayed;
			countText = (total == 1 ? tr("%2 of %1 entry shown (%3 filtered out)") : tr("%2 of %1 entries shown (%3 filtered out)")).arg(total).arg(displayed).arg(filtered);
		} else {
			countText = (total == 1 ? tr("%1 entry") : tr("%1 entries")).arg(total);
		}
		statusBarTableSizeLabel->setText(countText);
		
		QString filterText = QString();
		if (mapper.type == Ascent && !debugTable) {
			int filtersApplied = mapper.compTable->getCurrentFilters().size();
			if (filtersApplied) {
				filterText = (filtersApplied == 1 ? tr("%1 filter applied") : tr("%1 filters applied")).arg(filtersApplied);
			} else {
				filterText = tr("No filters applied");
			}
		}
		statusBarFiltersLabel->setText(filterText);
	});
	
	ascentCounterSegmentNumber->setProperty("value", QVariant(db.ascentsTable->getNumberOfRows()));
}



// EXECUTE USER COMMANDS

void MainWindow::viewItem(const ItemTypeMapper& mapper, int viewRowIndex)
{
	switch (mapper.type) {
	case Ascent:
		AscentViewer(this, &db, typesHandler, viewRowIndex).exec();
		return;
	default:
		assert(false);
	}
}

void MainWindow::newItem(const ItemTypeMapper& mapper)
{
	int newBufferRowIndex = mapper.openNewItemDialogAndStoreMethod(this, &db);
	if (newBufferRowIndex == -1) return;
	
	performUpdatesAfterUserAction(mapper, true, newBufferRowIndex);
	setStatusLine(tr("Saved new %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
}

void MainWindow::duplicateAndEditItem(const ItemTypeMapper& mapper, int viewRowIndex)
{
	int bufferRowIndex = mapper.compTable->getBufferRowIndexForViewRow(viewRowIndex);
	int newBufferRowIndex = mapper.openDuplicateItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	if (newBufferRowIndex == -1) return;
	
	performUpdatesAfterUserAction(mapper, true, newBufferRowIndex);
	setStatusLine(tr("Saved new %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
}

void MainWindow::editItem(const ItemTypeMapper& mapper, const QModelIndex& index)
{
	int bufferRowIndex = mapper.compTable->getBufferRowIndexForViewRow(index.row());
	mapper.openEditItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	
	performUpdatesAfterUserAction(mapper, false, bufferRowIndex);
	setStatusLine(tr("Saved changes in %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
}

void MainWindow::deleteItem(const ItemTypeMapper& mapper, int viewRowIndex)
{
	int bufferRowIndex = mapper.compTable->getBufferRowIndexForViewRow(viewRowIndex);
	mapper.openDeleteItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	
	performUpdatesAfterUserAction(mapper, true);
	setStatusLine(tr("Deleted %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
}


void MainWindow::performUpdatesAfterUserAction(const ItemTypeMapper& mapper, bool numberOfEntriesChanged, int bufferRowToSelectIndex)
{
	// Update selection in table
	if (bufferRowToSelectIndex >= 0) {
		int viewRowToSelectIndex = mapper.compTable->findViewRowIndexForBufferRow(bufferRowToSelectIndex);
		updateSelectionAfterUserAction(mapper, viewRowToSelectIndex);
	}
	// Update table size info
	if (numberOfEntriesChanged)	updateTableSize();
	// Update filters
	updateFilters();
}

void MainWindow::updateFilters(const ItemTypeMapper* mapper)
{
	if (!mapper || mapper->type == Range)	ascentFilterBar->updateRangeCombo();
	if (!mapper || mapper->type == Hiker)	ascentFilterBar->updateHikerCombo();
}

void MainWindow::updateSelectionAfterUserAction(const ItemTypeMapper& mapper, int viewRowIndex)
{
	QModelIndex modelIndex = mapper.compTable->index(viewRowIndex, 0);
	mapper.tableView->setCurrentIndex(modelIndex);
	mapper.tableView->scrollTo(modelIndex);
}



// UI EVENT HANDLERS

void MainWindow::handle_tabChanged()
{
	QProgressDialog progress(this);
	progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowCloseButtonHint);
	progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Updating table"));
	progress.setLabel(new QLabel(tr("Updating table..."), &progress));
	progress.setMinimumWidth(250);
	progress.setCancelButton(nullptr);
	progress.setMinimumDuration(500);
	
	QAbstractItemModel* currentModel = getCurrentTableView()->model();
	typesHandler->forEach([&currentModel, &progress] (const ItemTypeMapper& mapper) {
		CompositeTable* compTable = mapper.compTable;
		if (currentModel == compTable) {
			progress.setMaximum(mapper.compTable->getNumberOfCellsToUpdate());
			
			compTable->setUpdateImmediately(true, &progress);
		}
		else {
			compTable->setUpdateImmediately(false);
		}
	});
	
	updateTableSize();
}

void MainWindow::handle_rightClick(QPoint pos)
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex index = currentTableView->indexAt(pos);
	if (!index.isValid()) return;
	
	tableContextMenuOpenAction->setVisible(currentTableView == ascentsTableView);
	tableContextMenuDuplicateAction->setVisible(currentTableView == ascentsTableView || currentTableView == peaksTableView);
	
	tableContextMenu.popup(currentTableView->viewport()->mapToGlobal(pos));
}



// CONTEXT MENU ACTION HANDLERS

void MainWindow::handle_viewSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	bool done = typesHandler->forMatchingTableView(currentTableView, [this, selectedIndex] (const ItemTypeMapper& mapper, bool debugTable) {
		if (debugTable) {
			mapper.openEditItemDialogAndStoreMethod(this, &db, selectedIndex.row());
		} else if (mapper.type == Ascent) {
			viewItem(mapper, selectedIndex.row());
		} else {
			editItem(mapper, selectedIndex);
		}
	});
	assert(done);
}

void MainWindow::handle_editSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	bool done = typesHandler->forMatchingTableView(currentTableView, [this, selectedIndex] (const ItemTypeMapper& mapper, bool debugTable) {
		if (debugTable) {
			mapper.openEditItemDialogAndStoreMethod(this, &db, selectedIndex.row());
		} else {
			editItem(mapper, selectedIndex);
		}
	});
	assert(done);
}

void MainWindow::handle_duplicateAndEditSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	int viewRowIndex = selectedIndex.row();
	
	bool done = typesHandler->forMatchingTableView(currentTableView, [this, viewRowIndex] (const ItemTypeMapper& mapper, bool debugTable) {
		if (debugTable) {
			mapper.openDuplicateItemDialogAndStoreMethod(this, &db, viewRowIndex);
		} else {
			duplicateAndEditItem(mapper, viewRowIndex);
		}
	});
	assert(done);
}

void MainWindow::handle_deleteSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	int viewRowIndex = selectedIndex.row();
	
	bool done = typesHandler->forMatchingTableView(currentTableView, [this, viewRowIndex] (const ItemTypeMapper& mapper, bool debugTable) {
		if (debugTable) {
			mapper.openDeleteItemDialogAndStoreMethod(this, &db, viewRowIndex);
		} else {
			deleteItem(mapper, viewRowIndex);
		}
	});
	assert(done);
}



// FILE MENU ACTION HANDLERS

void MainWindow::handle_newDatabase()
{
	QString caption = tr("Save new database as");
	QString preSelectedDir = Settings::lastOpenDatabaseFile.get();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString filepath = QFileDialog::getSaveFileName(this, caption, preSelectedDir);
	if (filepath.isEmpty()) return;
	
	if (!filepath.endsWith(".db")) filepath.append(".db");
	
	handle_closeDatabase();
	
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

void MainWindow::handle_openDatabase()
{
	QString caption = tr("Open database");
	QString preSelectedDir = Settings::lastOpenDatabaseFile.get();
	if (preSelectedDir.isEmpty()) preSelectedDir = QDir::homePath();
	QString filter = tr("Database files") + " (*.db);;" + tr("All files") + " (*.*)";
	QString filepath = QFileDialog::getOpenFileName(this, caption, preSelectedDir, filter);
	if (filepath.isEmpty() || !QFile(filepath).exists()) return;
	
	handle_closeDatabase();
	
	setWindowTitleFilename(filepath);
	db.openExisting(this, filepath);
	updateFilters();
	initCompositeBuffers();
	updateTableSize();
	setUIEnabled(true);
	
	addToRecentFilesList(filepath);
}

void MainWindow::handle_openRecentDatabase(QString filepath)
{
	if (!QFile(filepath).exists()) {
		qDebug() << "Database file" << filepath << "was selected to be opened from recent files, but doesn't exist";
		return;
	}
	
	handle_closeDatabase();
	
	setWindowTitleFilename(filepath);
	db.openExisting(this, filepath);
	updateFilters();
	initCompositeBuffers();
	updateTableSize();
	setUIEnabled(true);
	
	addToRecentFilesList(filepath);
}

void MainWindow::handle_clearRecentDatabasesList()
{
	Settings::recentDatabaseFiles.set({ Settings::lastOpenDatabaseFile.get() });
	updateRecentFilesMenu();
}

void MainWindow::handle_saveDatabaseAs()
{
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

void MainWindow::handle_closeDatabase()
{
	setWindowTitleFilename();
	setUIEnabled(false);
	ascentFilterBar->resetUI();
	db.reset();
	updateFilters();
	typesHandler->forEach([] (const ItemTypeMapper& mapper) {
		mapper.compTable->resetBuffer();
	});
	updateTableSize(true);
}


void MainWindow::handle_openProjectSettings()
{
	ProjectSettingsWindow(this, &db).exec();
}

void MainWindow::handle_openSettings()
{
	SettingsWindow(this).exec();
}



// VIEW MENU ACTION HANDLERS

void MainWindow::handle_showFiltersChanged()
{
	bool showFilters = showFiltersAction->isChecked();
	ascentFilterBar->setVisible(showFilters);
	if (!showFilters) ascentFilterBar->handle_clearFilters();
}

void MainWindow::handle_autoResizeColumns()
{
	QTableView* currentTableView = getCurrentTableView();
	bool done = typesHandler->forMatchingTableView(currentTableView, [] (const ItemTypeMapper& mapper, bool debugTable) {
		QTableView* tableView = debugTable ? mapper.debugTableView : mapper.tableView;
		int numColumns = debugTable ? mapper.baseTable->getNumberOfColumns() : mapper.compTable->columnCount();
		
		tableView->resizeColumnsToContents();
		for (int i = 0; i < numColumns; i++) {
			if (tableView->columnWidth(i) > 400) tableView->setColumnWidth(i, 400);
		}
	});
	assert(done);
}



// TOOLS MENU ACTION HANDLERS

void MainWindow::handle_relocatePhotos()
{
	RelocatePhotosDialog(this, &db).exec();
}



// HELP MANU ACTION HANDLERS

void MainWindow::handle_about()
{
	AboutWindow(this).exec();
}



// CLOSING BEHAVIOUR

void MainWindow::closeEvent(QCloseEvent* event)
{
	saveImplicitSettings();
	
	QMainWindow::closeEvent(event);
}

void MainWindow::saveImplicitSettings() const
{
	bool maximized = windowState() == Qt::WindowMaximized;
	Settings::mainWindow_maximized.set(maximized);
	if (!maximized) Settings::mainWindow_geometry.set(geometry());
	
	Settings::mainWindow_currentTabIndex.set(mainAreaTabs->currentIndex());
	Settings::mainWindow_showFilters.set(showFiltersAction->isChecked());
	
	typesHandler->forEach([this] (const ItemTypeMapper& mapper) {
		saveColumnWidths(mapper);
		saveSorting(mapper);
	});
}

void MainWindow::saveColumnWidths(const ItemTypeMapper& mapper) const
{
	QStringList columnWidths;
	for (int i = 0; i < mapper.compTable->columnCount(); i++) {
		int currentColumnWidth = mapper.tableView->columnWidth(i);
		if (currentColumnWidth == 0) {
			qDebug() << "Couldn't read column width for column" << i << "in table" << mapper.compTable->name << "- skipping table";
			return;
		}
		columnWidths.append(QString::number(currentColumnWidth));
	}
	mapper.columnWidthsSetting->set(columnWidths);
}

void MainWindow::saveSorting(const ItemTypeMapper& mapper) const
{
	QPair<const CompositeColumn*, Qt::SortOrder> currentSorting = mapper.compTable->getCurrentSorting();
	int columnIndex = currentSorting.first->getIndex();
	Qt::SortOrder order = currentSorting.second;
	QString orderString = order == Qt::DescendingOrder ? "Descending" : "Ascending";
	mapper.sortingSetting->set({QString::number(columnIndex), orderString});
}



// GENERAL HELPERS

QTableView* MainWindow::getCurrentTableView() const
{
	return mainAreaTabs->currentWidget()->findChild<QTableView*>();
}

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

void MainWindow::setWindowTitleFilename(QString filepath)
{
	QString windowTitle = "PeakAscentLogger";
	if (!filepath.isEmpty()) {
		QString filename = QFileInfo(filepath).fileName();
		windowTitle += "   –   " + filename;
	}
	setWindowTitle(windowTitle);
}

void MainWindow::setStatusLine(QString content)
{
	statusbar->showMessage(content, 5000);
}
