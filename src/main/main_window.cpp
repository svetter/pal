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
		ui(new Ui::MainWindow),
		db(Database(this))
{
	ui->setupUi(this);
	
	connect(ui->newAscentAction,	&QAction::triggered,	this,	&MainWindow::handle_newAscent);
	connect(ui->newPeakAction,		&QAction::triggered,	this,	&MainWindow::handle_newPeak);
	connect(ui->newTripAction,		&QAction::triggered,	this,	&MainWindow::handle_newTrip);
	connect(ui->newHikerAction,		&QAction::triggered,	this,	&MainWindow::handle_newHiker);
	connect(ui->newRegionAction,	&QAction::triggered,	this,	&MainWindow::handle_newRegion);
	connect(ui->newRangeAction,		&QAction::triggered,	this,	&MainWindow::handle_newRange);
	connect(ui->newCountryAction,	&QAction::triggered,	this,	&MainWindow::handle_newCountry);
	
	connect(ui->newAscentButton,	&QPushButton::clicked,	this,	&MainWindow::handle_newAscent);
	connect(ui->newPeakButton,		&QPushButton::clicked,	this,	&MainWindow::handle_newPeak);
	connect(ui->newTripButton,		&QPushButton::clicked,	this,	&MainWindow::handle_newTrip);
	
	
	ui->numAscentsLcdNumber->setProperty("value", QVariant(db.getNumberOfEntries(db.ascentsTable)));
	updateHikersView();
}

MainWindow::~MainWindow()
{
	delete ui;
}



void MainWindow::updateHikersView()
{
	QList<Hiker*>* hikers = db.getAllHikers();
	
	QStandardItemModel* model = new QStandardItemModel();	
	ui->hikersTableView->setModel(model);
	
	int row = 0;
	for(auto iter = hikers->begin(); iter != hikers->end(); iter++) {
		model->setItem(row, 0, new QStandardItem(QString::number((*iter)->hikerID)));
		model->setItem(row, 1, new QStandardItem((*iter)->name));
		++row;
	}
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
	updateHikersView();
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