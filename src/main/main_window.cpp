#include "main_window.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/about_window.h"
#include "src/main/item_types_handler.h"
#include "src/main/project_settings_window.h"
#include "src/main/settings_window.h"
#include "src/tools/relocate_photos_dialog.h"
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
		typesHandler(nullptr)
{
	setupUi(this);
	statusbar->addPermanentWidget(statusBarTableSizeLabel);
	statusbar->addPermanentWidget(statusBarFiltersLabel);
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
	
	
	ascentFilterBar->supplyPointers(this, &db, (CompositeAscentsTable*) typesHandler->get(Ascent)->compTable);
	
	
	connectUI();
	setupTableViews();
	initTableContextMenuAndShortcuts();
	updateRecentFilesMenu();
	
	handle_showFiltersChanged();
	
	// Open database
	QString lastOpen = Settings::lastOpenDatabaseFile.get();
	if (!lastOpen.isEmpty() && QFile(lastOpen).exists()) {
		db.openExisting(this, lastOpen);
		setVisible(true);
		initCompositeBuffers();
		updateTableSize();
		setUIEnabled(true);
	}
	
	
	// Temporary
	setupDebugTableViews();
	
	// Temporary: Add menu item to insert test data into current database
	toolsMenu->addSeparator();
	toolsMenu->addAction("Insert test data", this, [=](){ db.insertTestData(this); updateTableSize(); });
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
		auto editFunction = [this, &mapper] (const QModelIndex& index) {
			editItem(mapper, index);
		};
		connect(mapper.tableView,			&QTableView::doubleClicked,		this,	editFunction);
		
		auto editFunctionDebug = [this, &mapper] (const QModelIndex& index) {
			mapper.openEditItemDialogAndStoreMethod(this, &db, index.row());
		};
		connect(mapper.debugTableView,		&QTableView::doubleClicked,		this,	editFunctionDebug);
	});
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
		numCells += mapper.compTable->getNumberOfCellsToInit();
	});
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
	
	typesHandler->forEach([&progress, &ascentFilters] (const ItemTypeMapper& mapper) {
		progress.setLabelText(tr("Preparing table %1...").arg(mapper.baseTable->uiName));
		
		if (mapper.type == Ascent) {
			mapper.compTable->initBuffer(&progress, ascentFilters);
		} else {
			mapper.compTable->initBuffer(&progress);
		}
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

void MainWindow::newItem(const ItemTypeMapper& mapper)
{
	int newBufferRowIndex = mapper.openNewItemDialogAndStoreMethod(this, &db);
	if (newBufferRowIndex == -1) return;
	
	int viewRowIndex = mapper.compTable->findCurrentViewRowIndex(newBufferRowIndex);
	updateSelectionAfterUserAction(mapper, viewRowIndex);
	updateTableSize();
	setStatusLine(tr("Saved new %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
}

void MainWindow::duplicateAndEditItem(const ItemTypeMapper& mapper, int viewRowIndex)
{
	int bufferRowIndex = mapper.compTable->getBufferRowForViewRow(viewRowIndex);
	int newBufferRowIndex = mapper.openDuplicateItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	if (newBufferRowIndex == -1) return;
	
	int newViewRowIndex = mapper.compTable->findCurrentViewRowIndex(newBufferRowIndex);
	updateSelectionAfterUserAction(mapper, newViewRowIndex);
	updateTableSize();
	setStatusLine(tr("Saved new %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
}

void MainWindow::editItem(const ItemTypeMapper& mapper, const QModelIndex& index)
{
	int bufferRowIndex = mapper.compTable->getBufferRowForViewRow(index.row());
	mapper.openEditItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	
	int viewRowIndex = mapper.compTable->updateSortingAfterItemEdit(index.row());	// TODO also update filtering
	updateSelectionAfterUserAction(mapper, viewRowIndex);
	setStatusLine(tr("Saved changes in %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
}

void MainWindow::deleteItem(const ItemTypeMapper& mapper, int viewRowIndex)
{
	int bufferRowIndex = mapper.compTable->getBufferRowForViewRow(viewRowIndex);
	mapper.openDeleteItemDialogAndStoreMethod(this, &db, bufferRowIndex);
	updateTableSize();
	setStatusLine(tr("Deleted %1.").arg(mapper.baseTable->getItemNameSingularLowercase()));
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
	db.createNew(this, filepath);
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
	db.openExisting(this, filepath);
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
	db.openExisting(this, filepath);
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
	}
	
	addToRecentFilesList(filepath);
}

void MainWindow::handle_closeDatabase()
{
	setUIEnabled(false);
	ascentFilterBar->resetUI();
	db.reset();
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



// DATABASE CALLBACK

void MainWindow::setStatusLine(QString content)
{
	statusbar->showMessage(content, 5000);
}
