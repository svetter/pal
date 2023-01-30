#include "main_window.h"
#include "src/dialogs/new_ascent.h"
#include "src/dialogs/new_country.h"
#include "src/dialogs/new_hiker.h"
#include "src/dialogs/new_peak.h"
#include "src/dialogs/new_range.h"
#include "src/dialogs/new_region.h"
#include "src/dialogs/new_trip.h"
#include "ui_main_window.h"



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
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
}

MainWindow::~MainWindow()
{
	delete ui;
}



void MainWindow::handle_newAscent()
{
	NewAscentDialog dialog(this);
    dialog.exec();
	//if (dialog.exec() == QDialog::Accepted)
	//	m_store->setCookie(dialog.cookie());
}

void MainWindow::handle_newPeak()
{
	NewPeakDialog dialog(this);
    dialog.exec();
}

void MainWindow::handle_newTrip()
{
	NewTripDialog dialog(this);
    dialog.exec();
}

void MainWindow::handle_newHiker()
{
	NewHikerDialog dialog(this);
    dialog.exec();
}

void MainWindow::handle_newRegion()
{
	NewRegionDialog dialog(this);
    dialog.exec();
}

void MainWindow::handle_newRange()
{
	NewRangeDialog dialog(this);
    dialog.exec();
}

void MainWindow::handle_newCountry()
{
	NewCountryDialog dialog(this);
    dialog.exec();
}