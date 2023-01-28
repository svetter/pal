#include "mainwindow.h"
#include "addascent.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	
	
	
	connect(ui->action_newAscent, &QAction::triggered, this, &MainWindow::handleNewAscentClicked);
	connect(ui->toolButton_addAscent, &QPushButton::clicked, this, &MainWindow::handleNewAscentClicked);
}

MainWindow::~MainWindow()
{
	delete ui;
}



void MainWindow::handleNewAscentClicked()
{
    AddAscentDialog dialog(this);
    dialog.exec();
	//if (dialog.exec() == QDialog::Accepted)
	//	m_store->setCookie(dialog.cookie());
}