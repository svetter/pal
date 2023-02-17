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
		tableContextMenu(QMenu(this))
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
	
	initTableContextMenu();
}

MainWindow::~MainWindow()
{}



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

void MainWindow::initTableContextMenu()
{
	QAction* openAction			= tableContextMenu.addAction(tr("Open..."),						QKeySequence(Qt::Key_Enter));
	tableContextMenu.addSeparator();
	QAction* editAction			= tableContextMenu.addAction(tr("Edit..."),						QKeySequence(Qt::CTRL | Qt::Key_Enter));
	QAction* duplicateAction	= tableContextMenu.addAction(tr("Edit as new duplicate..."),	QKeySequence::Copy);
	tableContextMenu.addSeparator();
	QAction* deleteAction		= tableContextMenu.addAction(tr("Delete..."),					QKeySequence::Delete);
	
	connect(openAction,			&QAction::triggered,	this,	&MainWindow::handle_contextMenu_openItem);
	connect(editAction,			&QAction::triggered,	this,	&MainWindow::handle_contextMenu_editItem);
	connect(duplicateAction,	&QAction::triggered,	this,	&MainWindow::handle_contextMenu_editDuplicatedItem);
	connect(deleteAction,		&QAction::triggered,	this,	&MainWindow::handle_contextMenu_deleteItem);
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



void MainWindow::handle_rightClick(QPoint pos)
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex index = currentTableView->indexAt(pos);
	if (!index.isValid()) return;
	
	tableContextMenu.popup(currentTableView->viewport()->mapToGlobal(pos));
}



void MainWindow::handle_contextMenu_openItem()
{
	QTableView* currentTableView = getCurrentTableView();
	NormalTable* currentTableModel = (NormalTable*) currentTableView->model();
	int selectedRowIndex = currentTableView->currentIndex().row();
	int itemID = currentTableModel->getBufferRow(selectedRowIndex)->at(0).toInt();
	qDebug() << QString("User requested to open %1 with ID=%2").arg(currentTableModel->getItemNameSingularLowercase()).arg(itemID);
	// TODO
}

void MainWindow::handle_contextMenu_editItem()
{
	QTableView* currentTableView = getCurrentTableView();
	QModelIndex selectedIndex = currentTableView->currentIndex();
	if (currentTableView == ascentsTableView)	{ handle_editAscent		(selectedIndex);	return; }
	if (currentTableView == peaksTableView)		{ handle_editPeak		(selectedIndex);	return; }
	if (currentTableView == tripsTableView)		{ handle_editTrip		(selectedIndex);	return; }
	if (currentTableView == hikersTableView)	{ handle_editHiker		(selectedIndex);	return; }
	if (currentTableView == regionsTableView)	{ handle_editRegion		(selectedIndex);	return; }
	if (currentTableView == rangesTableView)	{ handle_editRange		(selectedIndex);	return; }
	if (currentTableView == countriesTableView)	{ handle_editCountry	(selectedIndex);	return; }
	assert(false);
}

void MainWindow::handle_contextMenu_editDuplicatedItem()
{
	QTableView* currentTableView = getCurrentTableView();
	NormalTable* currentTableModel = (NormalTable*) currentTableView->model();
	int selectedRowIndex = currentTableView->currentIndex().row();
	int itemID = currentTableModel->getBufferRow(selectedRowIndex)->at(0).toInt();
	qDebug() << QString("User requested to duplicate and edit %1 with ID=%2").arg(currentTableModel->getItemNameSingularLowercase()).arg(itemID);
	// TODO
}

void MainWindow::handle_contextMenu_deleteItem()
{
	QTableView* currentTableView = getCurrentTableView();
	NormalTable* currentTableModel = (NormalTable*) currentTableView->model();
	int selectedRowIndex = currentTableView->currentIndex().row();
	int itemID = currentTableModel->getBufferRow(selectedRowIndex)->at(0).toInt();
	qDebug() << QString("User requested to delete %1 with ID=%2").arg(currentTableModel->getItemNameSingularLowercase()).arg(itemID);
	// TODO
}



QTableView* MainWindow::getCurrentTableView() const
{
	return mainAreaTabs->currentWidget()->findChild<QTableView*>();
}
