#include "main_window.h"
#include "src/dialogs/ascent_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/region_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "ui_main_window.h"

#include <QList>
#include <QStandardItemModel>



MainWindow::MainWindow() :
		QMainWindow(nullptr),
		db(Database(this)),
		tableContextMenu(QMenu(this)), tableContextMenuOpenAction(nullptr), tableContextMenuDuplicateAction(nullptr),
		shortcuts(QList<QShortcut*>())
{
	setupUi(this);
	
	connect(newAscentAction,	&QAction::triggered,	this,	&MainWindow::handle_newAscent);
	connect(newPeakAction,		&QAction::triggered,	this,	&MainWindow::handle_newPeak);
	connect(newTripAction,		&QAction::triggered,	this,	&MainWindow::handle_newTrip);
	connect(newHikerAction,		&QAction::triggered,	this,	&MainWindow::handle_newHiker);
	connect(newRegionAction,	&QAction::triggered,	this,	&MainWindow::handle_newRegion);
	connect(newRangeAction,		&QAction::triggered,	this,	&MainWindow::handle_newRange);
	connect(newCountryAction,	&QAction::triggered,	this,	&MainWindow::handle_newCountry);
	
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
	
	connect(ascentsTableView,	&QTableView::customContextMenuRequested,	this,	&MainWindow::handle_rightClick);
	connect(peaksTableView,		&QTableView::customContextMenuRequested,	this,	&MainWindow::handle_rightClick);
	connect(tripsTableView,		&QTableView::customContextMenuRequested,	this,	&MainWindow::handle_rightClick);
	connect(hikersTableView,	&QTableView::customContextMenuRequested,	this,	&MainWindow::handle_rightClick);
	connect(regionsTableView,	&QTableView::customContextMenuRequested,	this,	&MainWindow::handle_rightClick);
	connect(rangesTableView,	&QTableView::customContextMenuRequested,	this,	&MainWindow::handle_rightClick);
	connect(countriesTableView,	&QTableView::customContextMenuRequested,	this,	&MainWindow::handle_rightClick);
	
	
	updateAscentCounter();
	
	setupTableView(ascentsTableView,	db.ascentsTable);
	setupTableView(peaksTableView,		db.peaksTable);
	setupTableView(tripsTableView,		db.tripsTable);
	setupTableView(hikersTableView,		db.hikersTable);
	setupTableView(regionsTableView,	db.regionsTable);
	setupTableView(rangesTableView,		db.rangesTable);
	setupTableView(countriesTableView,	db.countriesTable);
	
	
	db.setStatusBar(statusbar);
	
	initTableContextMenuAndShortcuts();
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
	numAscentsLcdNumber->setProperty("value", QVariant(db.ascentsTable->getNumberOfRows()));
}



void MainWindow::setupTableView(QTableView* view, NormalTable* table)
{
	view->setModel(table);
	view->setRootIndex(table->getNormalRootModelIndex());
	view->resizeColumnsToContents();
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
	QModelIndex modelIndex = table->index(newItemIndex, 0, view->currentIndex().parent());
	view->setCurrentIndex(modelIndex);
	view->scrollTo(modelIndex);
}



void MainWindow::handle_openAscent(const QModelIndex& index)
{
	// TODO open ascent display window
	qDebug() << "UNIMPLEMENTED: MainWindow::handle_openAscent()";
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
	
	QModelIndex modelIndex = db.ascentsTable->index(newAscentIndex, 0, ascentsTableView->currentIndex().parent());
	ascentsTableView->setCurrentIndex(modelIndex);
	ascentsTableView->scrollTo(modelIndex);
	updateAscentCounter();
}

void MainWindow::handle_duplicateAndEditPeak(int rowIndex)
{
	Peak* selectedPeak = db.getPeakAt(rowIndex);
	int newPeakIndex = openDuplicatePeakDialogAndStore(this, &db, selectedPeak);
	
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



QTableView* MainWindow::getCurrentTableView() const
{
	return mainAreaTabs->currentWidget()->findChild<QTableView*>();
}
