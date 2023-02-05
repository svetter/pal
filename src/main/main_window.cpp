#include "main_window.h"
#include "src/dialogs/ascent_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/peak_dialog.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/region_dialog.h"
#include "src/dialogs/trip_dialog.h"
#include "ui_main_window.h"



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
}

MainWindow::~MainWindow()
{
	delete ui;
}



void MainWindow::handle_newAscent()
{
	openNewAscentDialogAndStore(this);
}

void MainWindow::handle_newPeak()
{
	openNewPeakDialogAndStore(this);
}

void MainWindow::handle_newTrip()
{
	openNewTripDialogAndStore(this);
}

void MainWindow::handle_newHiker()
{
	openNewHikerDialogAndStore(this);
}

void MainWindow::handle_newRegion()
{
	openNewRegionDialogAndStore(this);
}

void MainWindow::handle_newRange()
{
	openNewRangeDialogAndStore(this);
}

void MainWindow::handle_newCountry()
{
	openNewCountryDialogAndStore(this);
}