#include "main_window.h"

#include "src/main/about_window.h"
#include "src/main/item_types_handler.h"
#include "src/main/project_settings_window.h"
#include "src/main/settings_window.h"
#include "src/tools/relocate_photos_dialog.h"
#include "ui_main_window.h"

#include <QList>
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
		typesHandler(nullptr)
{
	setupUi(this);
	setUIEnabled(false);
	
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
	
	
	db.setStatusBar(statusbar);
	
	
	typesHandler = new ItemTypesHandler(
		new AscentTypeMapper	(&db, ascentsTab,	ascentsTableView,	ascentsDebugTableView,		newAscentAction,	newAscentButton),
		new PeakTypeMapper		(&db, peaksTab,		peaksTableView,		peaksDebugTableView,		newPeakAction,		newPeakButton),
		new TripTypeMapper		(&db, tripsTab,		tripsTableView,		tripsDebugTableView,		newTripAction,		newTripButton),
		new HikerTypeMapper		(&db, hikersTab,	hikersTableView,	hikersDebugTableView,		newHikerAction,		nullptr),
		new RegionTypeMapper	(&db, regionsTab,	regionsTableView,	regionsDebugTableView,		newRegionAction,	nullptr),
		new RangeTypeMapper		(&db, rangesTab,	rangesTableView,	rangesDebugTableView,		newRangeAction,		nullptr),
		new CountryTypeMapper	(&db, countriesTab,	countriesTableView,	countriesDebugTableView,	newCountryAction,	nullptr)
	);
	
	
	connectUI();
	setupTableViews();
	initTableContextMenuAndShortcuts();
	updateRecentFilesMenu();
	updateFilterUIEnabled();
	handle_showFiltersChanged();
	
	// Open database
	QString lastOpen = Settings::lastOpenDatabaseFile.get();
	if (!lastOpen.isEmpty() && QFile(lastOpen).exists()) {
		db.openExisting(this, lastOpen);
		setVisible(true);
		initCompositeBuffers();
		updateAscentCounter();
		setUIEnabled(true);
	}
	
	
	// Temporary
	setupDebugTableViews();
	
	// Temporary: Add menu item to insert test data into current database
	toolsMenu->addSeparator();
	toolsMenu->addAction("Insert test data", this, [=](){ db.insertTestData(this); updateAscentCounter(); });
}

MainWindow::~MainWindow()
{
	qDeleteAll(shortcuts);
	delete typesHandler;
}



// INITIAL SETUP

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
	connect(showFiltersAction,				&QAction::triggered,			this,	&MainWindow::handle_showFiltersChanged);
	// Menu "New"
	typesHandler->forEach([this] (const ItemTypeMapper& mapper) {
		auto newFunction = [this, &mapper] () {
			newItem(mapper.openNewItemDialogAndStoreMethod, mapper.compTable, mapper.tableView);
			if (mapper.type == Ascent) updateAscentCounter();
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
		auto editFunction = [this, &mapper] (const QModelIndex& index) {
			editItem(mapper.openEditItemDialogAndStoreMethod, mapper.compTable, mapper.tableView, index);
		};
		connect(mapper.tableView,			&QTableView::doubleClicked,		this,	editFunction);
		
		auto editFunctionDebug = [this, &mapper] (const QModelIndex& index) {
			mapper.openEditItemDialogAndStoreMethod(this, &db, index.row());
		};
		connect(mapper.debugTableView,		&QTableView::doubleClicked,		this,	editFunctionDebug);
	});
	// Filters
	connect(applyFiltersButton,				&QPushButton::clicked,				this,	&MainWindow::handle_applyFilters);
	connect(clearFiltersButton,				&QPushButton::clicked,				this,	&MainWindow::handle_clearFilters);
	connect(dateFilterBox,					&QGroupBox::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(peakHeightFilterBox,			&QGroupBox::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(volcanoFilterBox,				&QGroupBox::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(rangeFilterBox,					&QGroupBox::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(hikeKindFilterBox,				&QGroupBox::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(difficultyFilterBox,			&QGroupBox::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(hikerFilterBox,					&QGroupBox::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(dateFilterMinWidget,			&QDateEdit::dateChanged,			this,	&MainWindow::handle_filtersChanged);
	connect(dateFilterMaxCheckbox,			&QCheckBox::stateChanged,			this,	&MainWindow::handle_filtersChanged);
	connect(dateFilterMaxWidget,			&QDateEdit::dateChanged,			this,	&MainWindow::handle_filtersChanged);
	connect(peakHeightFilterMinCombo,		&QComboBox::currentIndexChanged,	this,	&MainWindow::handle_filtersChanged);
	connect(peakHeightFilterMaxCheckbox,	&QCheckBox::stateChanged,			this,	&MainWindow::handle_filtersChanged);
	connect(peakHeightFilterMaxCombo,		&QComboBox::currentIndexChanged,	this,	&MainWindow::handle_filtersChanged);
	connect(volcanoFilterYesRadio,			&QRadioButton::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(volcanoFilterNoRadio,			&QRadioButton::clicked,				this,	&MainWindow::handle_filtersChanged);
	connect(rangeFilterCombo,				&QComboBox::currentIndexChanged,	this,	&MainWindow::handle_filtersChanged);
	connect(hikeKindFilterCombo,			&QComboBox::currentIndexChanged,	this,	&MainWindow::handle_filtersChanged);
	connect(difficultyFilterSystemCombo,	&QComboBox::currentIndexChanged,	this,	&MainWindow::handle_filtersChanged);
	connect(difficultyFilterGradeCombo,		&QComboBox::currentIndexChanged,	this,	&MainWindow::handle_filtersChanged);
	connect(hikerFilterCombo,				&QComboBox::currentIndexChanged,	this,	&MainWindow::handle_filtersChanged);
}

void MainWindow::setupTableViews()
{
	typesHandler->forEach([this] (const ItemTypeMapper& mapper) {
		// Set model
		mapper.tableView->setModel(mapper.compTable);
		
		setColumnWidths(mapper);
		
		// Enable context menu
		connect(mapper.tableView, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClick);
		
		mapper.compTable->setUpdateImmediately(mapper.tableView == getCurrentTableView());
		
		setSorting(mapper);
	});
}

void MainWindow::setupDebugTableViews()
{
	auto setupFunction = [this] (QTableView* view, Table* table) {
		// Set model
		view->setModel(table);
		view->setRootIndex(table->getNormalRootModelIndex());
		view->resizeColumnsToContents();
		if (view == ascentsDebugTableView && view->columnWidth(db.ascentsTable->descriptionColumn->getIndex()) > 400) {
			view->setColumnWidth(db.ascentsTable->descriptionColumn->getIndex(), 400);
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

void MainWindow::setColumnWidths(const ItemTypeMapper& mapper)
{
	if (!Settings::rememberColumnWidths.get()) {
		return mapper.tableView->resizeColumnsToContents();
	}
	
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
	QKeySequence openKeySequence		= QKeySequence(Qt::Key_Enter);
	QKeySequence editKeySequence		= QKeySequence(Qt::CTRL | Qt::Key_Enter);
	QKeySequence duplicateKeySequence	= QKeySequence::Copy;
	QKeySequence deleteKeySequence		= QKeySequence::Delete;
	
	// Context menu
	QAction* openAction			= tableContextMenu.addAction(tr("Open..."),						openKeySequence);
	tableContextMenu.addSeparator();
	QAction* editAction			= tableContextMenu.addAction(tr("Edit..."),						editKeySequence);
	QAction* duplicateAction	= tableContextMenu.addAction(tr("Edit as new duplicate..."),	duplicateKeySequence);
	tableContextMenu.addSeparator();
	QAction* deleteAction		= tableContextMenu.addAction(tr("Delete..."),					deleteKeySequence);
	// store actions for open and duplicate (for disbling them where they're not needed)
	tableContextMenuOpenAction		= openAction;
	tableContextMenuDuplicateAction	= duplicateAction;
	
	connect(openAction,			&QAction::triggered,	this,	&MainWindow::handle_openSelectedItem);
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
		
		connect(openShortcut,		&QShortcut::activated, this, &MainWindow::handle_openSelectedItem);
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
	
	int numCells = 0;
	typesHandler->forEach([&numCells] (const ItemTypeMapper& mapper) {
		numCells += mapper.compTable->getBaseTable()->getNumberOfRows() * mapper.compTable->columnCount();
	});
	progress.setMinimum(0);
	progress.setMaximum(numCells);
	progress.setValue(0);
	
	typesHandler->forEach([&progress] (const ItemTypeMapper& mapper) {
		progress.setLabelText(tr("Preparing table %1...").arg(mapper.baseTable->uiName));
		mapper.compTable->initBuffer(&progress);
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
	
	newMenu						->setEnabled(enabled);
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
		openRecentActions.append(newAction);
	}
	
	openRecentMenu->addSeparator();
	openRecentMenu->addAction(clearRecentDatabaseListAction);
	openRecentMenu->setEnabled(true);
}

void MainWindow::updateAscentCounter()
{
	ascentCounterSegmentNumber->setProperty("value", QVariant(db.ascentsTable->getNumberOfRows()));
}

void MainWindow::updateFilterUIEnabled()
{
	dateFilterMaxWidget			->setEnabled(dateFilterMaxCheckbox->checkState());
	peakHeightFilterMaxCombo	->setEnabled(peakHeightFilterMaxCheckbox->checkState());
	difficultyFilterGradeCombo	->setEnabled(difficultyFilterSystemCombo->currentIndex() > 0);
	
	if (!dateFilterMaxCheckbox->checkState()) {
		dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
	}
	else if (dateFilterMinWidget->date() > dateFilterMaxWidget->date()) {
		if (dateFilterMaxWidget->hasFocus() || dateFilterMaxWidget->calendarWidget()->hasFocus()) {
			dateFilterMinWidget->setDate(dateFilterMaxWidget->date());
		} else {
			dateFilterMaxWidget->setDate(dateFilterMinWidget->date());
		}
	}
	
	if (!peakHeightFilterMaxCheckbox->checkState()) {
		peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
	}
	else if (peakHeightFilterMinCombo->currentIndex() > peakHeightFilterMaxCombo->currentIndex()) {
		if (peakHeightFilterMaxCombo->hasFocus()) {
			peakHeightFilterMinCombo->setCurrentIndex(peakHeightFilterMaxCombo->currentIndex());
		} else {
			peakHeightFilterMaxCombo->setCurrentIndex(peakHeightFilterMinCombo->currentIndex());
		}
	}
}



// EXECUTE USER COMMANDS

void MainWindow::newItem(int (*openNewItemDialogAndStoreMethod) (QWidget*, Database*), CompositeTable* compTable, QTableView* tableView)
{
	int newBufferRowIndex = openNewItemDialogAndStoreMethod(this, &db);
	if (newBufferRowIndex == -1) return;
	
	int viewRowIndex = compTable->findCurrentViewRowIndex(newBufferRowIndex);
	updateSelectionAfterUserAction(tableView, compTable, viewRowIndex);
}

void MainWindow::duplicateAndEditItem(int (*openDuplicateItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, QTableView* tableView, int viewRowIndex)
{
	int bufferRowIndex = compTable->getBufferRowForViewRow(viewRowIndex);
	int newBufferRowIndex = openDuplicateItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	if (newBufferRowIndex == -1) return;
	
	int newViewRowIndex = compTable->findCurrentViewRowIndex(newBufferRowIndex);
	updateSelectionAfterUserAction(tableView, compTable, newViewRowIndex);
}

void MainWindow::editItem(void (*openEditItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, QTableView* tableView, const QModelIndex& index)
{
	int bufferRowIndex = compTable->getBufferRowForViewRow(index.row());
	openEditItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	
	int viewRowIndex = compTable->updateSortingAfterItemEdit(index.row());
	updateSelectionAfterUserAction(tableView, compTable, viewRowIndex);
}

void MainWindow::deleteItem(void (*openDeleteItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, int viewRowIndex)
{
	int bufferRowIndex = compTable->getBufferRowForViewRow(viewRowIndex);
	openDeleteItemDialogAndStoreMethod(this, &db, bufferRowIndex);
}


void MainWindow::updateSelectionAfterUserAction(QTableView* tableView, CompositeTable* compTable, int viewRowIndex)
{
	QModelIndex modelIndex = compTable->index(viewRowIndex, 0);
	tableView->setCurrentIndex(modelIndex);
	tableView->scrollTo(modelIndex);
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

void MainWindow::handle_openSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	if (currentTableView == ascentsTableView) {
		qDebug() << "UNIMPLEMENTED: MainWindow::handle_openAscent(), row" << selectedIndex.row();
		return;
	}
	qDebug() << "Missing implementation in MainWindow::handle_openSelectedItem() for" << currentTableView->objectName();
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
			editItem(mapper.openEditItemDialogAndStoreMethod, mapper.compTable, mapper.tableView, selectedIndex);
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
			duplicateAndEditItem(mapper.openDuplicateItemDialogAndStoreMethod, mapper.compTable, mapper.tableView, viewRowIndex);
		}
		if (mapper.type == Ascent) updateAscentCounter();
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
			deleteItem(mapper.openDeleteItemDialogAndStoreMethod, mapper.compTable, viewRowIndex);
		}
		if (mapper.type == Ascent) updateAscentCounter();
	});
	assert(done);
}



// FILTER EVENT HANDLERS

void MainWindow::handle_filtersChanged()
{
	updateFilterUIEnabled();
	applyFiltersButton->setEnabled(true);
}

void MainWindow::handle_applyFilters()
{
	// TODO
	applyFiltersButton->setEnabled(false);
	clearFiltersButton->setEnabled(true);
	qDebug() << "UNIMPLEMENTED: Apply filters";
}

void MainWindow::handle_clearFilters()
{
	// TODO
	clearFiltersButton->setEnabled(false);
	applyFiltersButton->setEnabled(true);
	qDebug() << "UNIMPLEMENTED: Clear filters";
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
	db.createNew(this, filepath);
	updateAscentCounter();
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
	db.openExisting(this, filepath);
	initCompositeBuffers();
	updateAscentCounter();
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
	db.openExisting(this, filepath);
	initCompositeBuffers();
	updateAscentCounter();
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
	
	if (!filepath.endsWith(".db")) filepath.append(".db");
	
	db.saveAs(this, filepath);
	
	addToRecentFilesList(filepath);
}

void MainWindow::handle_closeDatabase()
{
	setUIEnabled(false);
	db.reset();
	typesHandler->forEach([] (const ItemTypeMapper& mapper) {
		mapper.compTable->resetBuffer();
	});
	updateAscentCounter();
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
	filtersLayoutWidget->setVisible(showFilters);
	if (!showFilters) handle_clearFilters();
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
	
	typesHandler->forEach([] (const ItemTypeMapper& mapper) {
		// Save column widths
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
		
		// Save sorting
		QPair<const CompositeColumn*, Qt::SortOrder> currentSorting = mapper.compTable->getCurrentSorting();
		int columnIndex = currentSorting.first->getIndex();
		Qt::SortOrder order = currentSorting.second;
		QString orderString = order == Qt::DescendingOrder ? "Descending" : "Ascending";
		mapper.sortingSetting->set({QString::number(columnIndex), orderString});
	});
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



// DATABASE CALLBACK

void MainWindow::setStatusLine(QString content)
{
	statusbar->showMessage(content);
}
