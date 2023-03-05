#include "main_window.h"

#include "src/dialogs/ascent_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/region_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "src/main/about_window.h"
#include "src/main/project_settings_window.h"
#include "src/main/settings_window.h"
#include "src/tools/relocate_photos_dialog.h"
#include "ui_main_window.h"

#include <QList>
#include <QFileDialog>



MainWindow::MainWindow() :
		QMainWindow(nullptr),
		db(Database()),
		openRecentActions(QList<QAction*>()),
		tableContextMenu(QMenu(this)), tableContextMenuOpenAction(nullptr), tableContextMenuDuplicateAction(nullptr),
		shortcuts(QList<QShortcut*>()),
		compAscents		(CompositeAscentsTable(&db)),
		compPeaks		(CompositePeaksTable(&db)),
		compTrips		(CompositeTripsTable(&db)),
		compHikers		(CompositeHikersTable(&db)),
		compRegions		(CompositeRegionsTable(&db)),
		compRanges		(CompositeRangesTable(&db)),
		compCountries	(CompositeCountriesTable(&db))
{
	setupUi(this);
	setUIEnabled(false);
	
	setWindowState(Settings::mainWindow_maximized.get() ? Qt::WindowMaximized : Qt::WindowNoState);
	const QRect savedGeometry = Settings::mainWindow_geometry.get();
	if (!savedGeometry.isEmpty()) {
		setGeometry(savedGeometry);
	}
	
	if (Settings::mainWindow_rememberTab.get()) {
		mainAreaTabs->setCurrentIndex(Settings::mainWindow_currentTabIndex.get());
	}
	
	
	connect(newDatabaseAction,				&QAction::triggered,	this,	&MainWindow::handle_newDatabase);
	connect(openDatabaseAction,				&QAction::triggered,	this,	&MainWindow::handle_openDatabase);
	connect(clearRecentDatabaseListAction,	&QAction::triggered,	this,	&MainWindow::handle_clearRecentDatabasesList);
	connect(saveDatabaseAsAction,			&QAction::triggered,	this,	&MainWindow::handle_saveDatabaseAs);
	connect(closeDatabaseAction,			&QAction::triggered,	this,	&MainWindow::handle_closeDatabase);
	connect(projectSettingsAction,			&QAction::triggered,	this,	&MainWindow::handle_openProjectSettings);
	connect(settingsAction,					&QAction::triggered,	this,	&MainWindow::handle_openSettings);
	
	connect(newAscentAction,	&QAction::triggered,	this,	&MainWindow::handle_newAscent);
	connect(newPeakAction,		&QAction::triggered,	this,	&MainWindow::handle_newPeak);
	connect(newTripAction,		&QAction::triggered,	this,	&MainWindow::handle_newTrip);
	connect(newHikerAction,		&QAction::triggered,	this,	&MainWindow::handle_newHiker);
	connect(newRegionAction,	&QAction::triggered,	this,	&MainWindow::handle_newRegion);
	connect(newRangeAction,		&QAction::triggered,	this,	&MainWindow::handle_newRange);
	connect(newCountryAction,	&QAction::triggered,	this,	&MainWindow::handle_newCountry);
	
	connect(relocatePhotosAction,	&QAction::triggered,	this,	&MainWindow::handle_relocatePhotos);
	
	connect(aboutAction,		&QAction::triggered,	this,	&MainWindow::handle_about);
	connect(aboutQtAction,		&QAction::triggered,	this,	&QApplication::aboutQt);
	
	connect(newAscentButton,	&QPushButton::clicked,	this,	&MainWindow::handle_newAscent);
	connect(newPeakButton,		&QPushButton::clicked,	this,	&MainWindow::handle_newPeak);
	connect(newTripButton,		&QPushButton::clicked,	this,	&MainWindow::handle_newTrip);
	
	connect(ascentsTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editAscent);
	connect(peaksTableView,		&QTableView::doubleClicked,	this,	&MainWindow::handle_editPeak);
	connect(tripsTableView,		&QTableView::doubleClicked,	this,	&MainWindow::handle_editTrip);
	connect(hikersTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editHiker);
	connect(regionsTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editRegion);
	connect(rangesTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editRange);
	connect(countriesTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editCountry);
	
	
	updateAscentCounter();
	setUIEnabled(false);
	
	
	db.setStatusBar(statusbar);
	
	initTableContextMenuAndShortcuts();
	
	
	updateRecentFilesMenu();
	// Open database
	QString lastOpen = Settings::lastOpenDatabaseFile.get();
	if (!lastOpen.isEmpty() && QFile(lastOpen).exists()) {
		db.openExisting(this, lastOpen);
		updateAscentCounter();
		setUIEnabled(true);
	}
	
	
	setupTableView(ascentsTableView,	&compAscents,	&Settings::mainWindow_columnWidths_ascentsTable);
	setupTableView(peaksTableView,		&compPeaks,		&Settings::mainWindow_columnWidths_peaksTable);
	setupTableView(tripsTableView,		&compTrips,		&Settings::mainWindow_columnWidths_tripsTable);
	setupTableView(hikersTableView,		&compHikers,	&Settings::mainWindow_columnWidths_hikersTable);
	setupTableView(regionsTableView,	&compRegions,	&Settings::mainWindow_columnWidths_regionsTable);
	setupTableView(rangesTableView,		&compRanges,	&Settings::mainWindow_columnWidths_rangesTable);
	setupTableView(countriesTableView,	&compCountries,	&Settings::mainWindow_columnWidths_countriesTable);
	
	setupDebugTableView(ascentsDebugTableView,		db.ascentsTable);
	setupDebugTableView(peaksDebugTableView,		db.peaksTable);
	setupDebugTableView(tripsDebugTableView,		db.tripsTable);
	setupDebugTableView(hikersDebugTableView,		db.hikersTable);
	setupDebugTableView(regionsDebugTableView,		db.regionsTable);
	setupDebugTableView(rangesDebugTableView,		db.rangesTable);
	setupDebugTableView(countriesDebugTableView,	db.countriesTable);
	
	
	// Temporary: Add menu item to insert test data into current database
	toolsMenu->addSeparator();
	toolsMenu->addAction("Insert test data", this, [=](){ db.insertTestData(this); updateAscentCounter(); });
}

MainWindow::~MainWindow()
{
	qDeleteAll(shortcuts);
}



void MainWindow::setStatusLine(QString content)
{
	statusbar->showMessage(content);
}

void MainWindow::updateAscentCounter()
{
	ascentCounterSegmentNumber->setProperty("value", QVariant(db.ascentsTable->getNumberOfRows()));
}



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
	clearRecentFilesMenu();
	
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

void MainWindow::clearRecentFilesMenu()
{
	for (QAction* action : openRecentActions) {
		openRecentMenu->removeAction(action);
		// Don't clear, there's also a separator and the clear list action
	}
	openRecentActions.clear();
}


void MainWindow::setupTableView(QTableView* view, CompositeTable* table, const Setting<QStringList>* columnWidthsSetting)
{
	// Set model
	view->setModel(table);
	
	setColumnWidths(view, table, columnWidthsSetting);
	
	// Enable context menu
	connect(view, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClick);
}

void MainWindow::setupDebugTableView(QTableView* view, NormalTable* table)
{
	// Set model
	view->setModel(table);
	view->setRootIndex(table->getNormalRootModelIndex());
	
	// Enable context menu
	connect(view, &QTableView::customContextMenuRequested, this, &MainWindow::handle_rightClick);
}

void MainWindow::setColumnWidths(QTableView* view, const CompositeTable* table, const Setting<QStringList>* columnWidthsSetting)
{
	if (!Settings::mainWindow_rememberColumnWidths.get()) {
		return view->resizeColumnsToContents();
	}
	
	QStringList columnWidths = columnWidthsSetting->get();
	if (columnWidths.size() != table->columnCount()) {
		// Can't restore column widths from settings
		if (!columnWidths.isEmpty())
			qDebug() << QString("Couldn't restore column widths for table %1: Expected %2 numbers, but got %3")
					.arg(table->name).arg(table->columnCount()).arg(columnWidths.size());
		
		columnWidthsSetting->clear();
		return view->resizeColumnsToContents();
	}
	
	// Restore column widths
	for (int i = 0; i < table->columnCount(); i++) {
		view->setColumnWidth(i, columnWidths.at(i).toInt());
	}
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



void MainWindow::handle_openSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	if (currentTableView == ascentsTableView)	{ handle_openAscent		(selectedIndex);	return; }
	qDebug() << "Missing implementation in MainWindow::handle_openSelectedItem() for" << currentTableView->objectName();
}

void MainWindow::handle_editSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	
	if (currentTableView == ascentsTableView)	{ handle_editAscent		(selectedIndex);	return; }
	if (currentTableView == peaksTableView)		{ handle_editPeak		(selectedIndex);	return; }
	if (currentTableView == tripsTableView)		{ handle_editTrip		(selectedIndex);	return; }
	if (currentTableView == hikersTableView)	{ handle_editHiker		(selectedIndex);	return; }
	if (currentTableView == regionsTableView)	{ handle_editRegion		(selectedIndex);	return; }
	if (currentTableView == rangesTableView)	{ handle_editRange		(selectedIndex);	return; }
	if (currentTableView == countriesTableView)	{ handle_editCountry	(selectedIndex);	return; }
	qDebug() << "Missing implementation in MainWindow::handle_editSelectedItem() for" << currentTableView->objectName();
}

void MainWindow::handle_duplicateAndEditSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	int rowIndex = selectedIndex.row();
	
	if (currentTableView == ascentsTableView)	{ handle_duplicateAndEditAscent	(rowIndex);	return; }
	if (currentTableView == peaksTableView)		{ handle_duplicateAndEditPeak	(rowIndex);	return; }
	qDebug() << "Missing implementation in MainWindow::handle_duplicateAndEditSelectedItem() for" << currentTableView->objectName();
}

void MainWindow::handle_deleteSelectedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (!selectedIndex.isValid() || selectedIndex.row() < 0) return;
	int rowIndex = selectedIndex.row();
	
	if (currentTableView == ascentsTableView)	{ handle_deleteAscent	(rowIndex);	return; }
	if (currentTableView == peaksTableView)		{ handle_deletePeak		(rowIndex);	return; }
	if (currentTableView == tripsTableView)		{ handle_deleteTrip		(rowIndex);	return; }
	if (currentTableView == hikersTableView)	{ handle_deleteHiker	(rowIndex);	return; }
	if (currentTableView == regionsTableView)	{ handle_deleteRegion	(rowIndex);	return; }
	if (currentTableView == rangesTableView)	{ handle_deleteRange	(rowIndex);	return; }
	if (currentTableView == countriesTableView)	{ handle_deleteCountry	(rowIndex);	return; }
	qDebug() << "Missing implementation in MainWindow::handle_deleteSelectedItem() for" << currentTableView->objectName();
}



void MainWindow::handle_newAscent()
{
	handle_newItem(&openNewAscentDialogAndStore, db.ascentsTable, ascentsTableView);
	updateAscentCounter();
}

void MainWindow::handle_newPeak()
{
	handle_newItem(&openNewPeakDialogAndStore, db.peaksTable, peaksTableView);
}

void MainWindow::handle_newTrip()
{
	handle_newItem(&openNewTripDialogAndStore, db.tripsTable, tripsTableView);
}

void MainWindow::handle_newHiker()
{
	handle_newItem(&openNewHikerDialogAndStore, db.hikersTable, hikersTableView);
}

void MainWindow::handle_newRegion()
{
	handle_newItem(&openNewRegionDialogAndStore, db.regionsTable, regionsTableView);
}

void MainWindow::handle_newRange()
{
	handle_newItem(&openNewRangeDialogAndStore, db.rangesTable, rangesTableView);
}

void MainWindow::handle_newCountry()
{
	handle_newItem(&openNewCountryDialogAndStore, db.countriesTable, countriesTableView);
}

void MainWindow::handle_newItem(int (*openNewItemDialogAndStoreMethod) (QWidget*, Database*), NormalTable* table, QTableView* view)
{
	int newItemIndex = openNewItemDialogAndStoreMethod(this, &db);
	if (newItemIndex == -1) return;
	
	QModelIndex modelIndex = table->index(newItemIndex, 0, view->currentIndex().parent());
	view->setCurrentIndex(modelIndex);
	view->scrollTo(modelIndex);
}



void MainWindow::handle_openAscent(const QModelIndex& index)
{
	// TODO open ascent display window
	qDebug() << "UNIMPLEMENTED: MainWindow::handle_openAscent(), row" << index.row();
}



void MainWindow::handle_editAscent(const QModelIndex& index)
{
	Ascent* selectedAscent = db.getAscentAt(index.row());
	openEditAscentDialogAndStore(this, &db, selectedAscent);
}

void MainWindow::handle_editPeak(const QModelIndex& index)
{
	Peak* selectedPeak = db.getPeakAt(index.row());
	openEditPeakDialogAndStore(this, &db, selectedPeak);
}

void MainWindow::handle_editTrip(const QModelIndex& index)
{
	Trip* selectedTrip = db.getTripAt(index.row());
	openEditTripDialogAndStore(this, &db, selectedTrip);
}

void MainWindow::handle_editHiker(const QModelIndex& index)
{
	Hiker* selectedHiker = db.getHikerAt(index.row());
	openEditHikerDialogAndStore(this, &db, selectedHiker);
}

void MainWindow::handle_editRegion(const QModelIndex& index)
{
	Region* selectedRegion = db.getRegionAt(index.row());
	openEditRegionDialogAndStore(this, &db, selectedRegion);
}

void MainWindow::handle_editRange(const QModelIndex& index)
{
	Range* selectedRange = db.getRangeAt(index.row());
	openEditRangeDialogAndStore(this, &db, selectedRange);
}

void MainWindow::handle_editCountry(const QModelIndex& index)
{
	Country* selectedCountry = db.getCountryAt(index.row());
	openEditCountryDialogAndStore(this, &db, selectedCountry);
}



void MainWindow::handle_duplicateAndEditAscent(int rowIndex)
{
	Ascent* selectedAscent = db.getAscentAt(rowIndex);
	int newAscentIndex = openDuplicateAscentDialogAndStore(this, &db, selectedAscent);
	if (newAscentIndex == -1) return;
	
	QModelIndex modelIndex = db.ascentsTable->index(newAscentIndex, 0, ascentsTableView->currentIndex().parent());
	ascentsTableView->setCurrentIndex(modelIndex);
	ascentsTableView->scrollTo(modelIndex);
	updateAscentCounter();
}

void MainWindow::handle_duplicateAndEditPeak(int rowIndex)
{
	Peak* selectedPeak = db.getPeakAt(rowIndex);
	int newPeakIndex = openDuplicatePeakDialogAndStore(this, &db, selectedPeak);
	if (newPeakIndex == -1) return;
	
	QModelIndex modelIndex = db.peaksTable->index(newPeakIndex, 0, peaksTableView->currentIndex().parent());
	peaksTableView->setCurrentIndex(modelIndex);
	peaksTableView->scrollTo(modelIndex);
}



void MainWindow::handle_deleteAscent(int rowIndex)
{
	Ascent* selectedAscent = db.getAscentAt(rowIndex);
	openDeleteAscentDialogAndExecute(this, &db, selectedAscent);
	updateAscentCounter();
}

void MainWindow::handle_deletePeak(int rowIndex)
{
	Peak* selectedPeak = db.getPeakAt(rowIndex);
	openDeletePeakDialogAndExecute(this, &db, selectedPeak);
}

void MainWindow::handle_deleteTrip(int rowIndex)
{
	Trip* selectedTrip = db.getTripAt(rowIndex);
	openDeleteTripDialogAndExecute(this, &db, selectedTrip);
}

void MainWindow::handle_deleteHiker(int rowIndex)
{
	Hiker* selectedHiker = db.getHikerAt(rowIndex);
	openDeleteHikerDialogAndExecute(this, &db, selectedHiker);
}

void MainWindow::handle_deleteRegion(int rowIndex)
{
	Region* selectedRegion = db.getRegionAt(rowIndex);
	openDeleteRegionDialogAndExecute(this, &db, selectedRegion);
}

void MainWindow::handle_deleteRange(int rowIndex)
{
	Range* selectedRange = db.getRangeAt(rowIndex);
	openDeleteRangeDialogAndExecute(this, &db, selectedRange);
}

void MainWindow::handle_deleteCountry(int rowIndex)
{
	Country* selectedCountry = db.getCountryAt(rowIndex);
	openDeleteCountryDialogAndExecute(this, &db, selectedCountry);
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



void MainWindow::handle_relocatePhotos()
{
	RelocatePhotosDialog(this, &db).exec();
}



void MainWindow::handle_about()
{
	AboutWindow(this).exec();
}



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
	
	saveColumnWidths(ascentsTableView,		&compAscents,	&Settings::mainWindow_columnWidths_ascentsTable);
	saveColumnWidths(peaksTableView,		&compPeaks,		&Settings::mainWindow_columnWidths_peaksTable);
	saveColumnWidths(tripsTableView,		&compTrips,		&Settings::mainWindow_columnWidths_tripsTable);
	saveColumnWidths(hikersTableView,		&compHikers,	&Settings::mainWindow_columnWidths_hikersTable);
	saveColumnWidths(regionsTableView,		&compRegions,	&Settings::mainWindow_columnWidths_regionsTable);
	saveColumnWidths(rangesTableView,		&compRanges,	&Settings::mainWindow_columnWidths_rangesTable);
	saveColumnWidths(countriesTableView,	&compCountries,	&Settings::mainWindow_columnWidths_countriesTable);
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



QTableView* MainWindow::getCurrentTableView() const
{
	return mainAreaTabs->currentWidget()->findChild<QTableView*>();
}



void MainWindow::saveColumnWidths(QTableView* view, const CompositeTable* table, const Setting<QStringList>* columnWidthsSetting) const
{
	QStringList columnWidths;
	for (int i = 0; i < table->columnCount(); i++) {
		columnWidths.append(QString::number(view->columnWidth(i)));
	}
	columnWidthsSetting->set(columnWidths);
}
