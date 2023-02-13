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
		db(Database(this))
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
	connect(peaksTableView,		&QTableView::doubleClicked,	this,	&MainWindow::handle_editAscent);
	connect(tripsTableView,		&QTableView::doubleClicked,	this,	&MainWindow::handle_editAscent);
	connect(hikersTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editAscent);
	connect(regionsTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editAscent);
	connect(rangesTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editAscent);
	connect(countriesTableView,	&QTableView::doubleClicked,	this,	&MainWindow::handle_editAscent);
	
	numAscentsLcdNumber->setProperty("value", QVariant(db.ascentsTable->getNumberOfEntries(this)));
	
	setupTableView(ascentsTableView,	db.ascentsTable);
	setupTableView(peaksTableView,		db.peaksTable);
	setupTableView(tripsTableView,		db.tripsTable);
	setupTableView(hikersTableView,		db.hikersTable);
	setupTableView(regionsTableView,	db.regionsTable);
	setupTableView(rangesTableView,		db.rangesTable);
	setupTableView(countriesTableView,	db.countriesTable);
}

MainWindow::~MainWindow()
{}



void MainWindow::setupTableView(QTableView* view, NormalTable* table)
{
	view->setModel(table);
	view->setRootIndex(table->getNormalRootModelIndex());
	view->resizeColumnsToContents();
}



void MainWindow::handle_newAscent()
{
	openNewAscentDialogAndStore(this, &db);
	// TODO update active view if necessary
}

void MainWindow::handle_newPeak()
{
	openNewPeakDialogAndStore(this, &db);
	// TODO update active view if necessary
}

void MainWindow::handle_newTrip()
{
	openNewTripDialogAndStore(this, &db);
	// TODO update active view if necessary
}

void MainWindow::handle_newHiker()
{
	openNewHikerDialogAndStore(this, &db);
	// TODO update active view if necessary
}

void MainWindow::handle_newRegion()
{
	openNewRegionDialogAndStore(this, &db);
	// TODO update active view if necessary
}

void MainWindow::handle_newRange()
{
	openNewRangeDialogAndStore(this, &db);
	// TODO update active view if necessary
}

void MainWindow::handle_newCountry()
{
	openNewCountryDialogAndStore(this, &db);
	// TODO update active view if necessary
}


void MainWindow::handle_editAscent(const QModelIndex& index)
{
	Ascent* selectedAscent = db.getAscentAt(index.row());
	openEditAscentDialogAndStore(this, &db, selectedAscent);
}

void MainWindow::handle_editPeak(const QModelIndex& index)
{
	//openEditPeakDialogAndStore(this, &db, selectedPeak);
}

void MainWindow::handle_editTrip(const QModelIndex& index)
{
	//openEditTripDialogAndStore(this, &db, selectedTrip);
}

void MainWindow::handle_editHiker(const QModelIndex& index)
{
	//openEditHikerDialogAndStore(this, &db, selectedHiker);
}

void MainWindow::handle_editRegion(const QModelIndex& index)
{
	//openEditRegionDialogAndStore(this, &db, selectedRegion);
}

void MainWindow::handle_editRange(const QModelIndex& index)
{
	//openEditRangeDialogAndStore(this, &db, selectedRange);
}

void MainWindow::handle_editCountry(const QModelIndex& index)
{
	//openEditCountryDialogAndStore(this, &db, selectedCountry);
}
