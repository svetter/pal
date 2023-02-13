#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "src/db/db_interface.h"
#include "ui_main_window.h"

#include <QMainWindow>
#include <QTableView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow, public Ui_MainWindow
{
	Q_OBJECT
	
public:
	MainWindow();
	~MainWindow();
	
private:
	Database db;
	
	void setupTableView(QTableView* view, NormalTable* table);
	
private slots:
	void handle_newAscent();
	void handle_newPeak();
	void handle_newTrip();
	void handle_newHiker();
	void handle_newRegion();
	void handle_newRange();
	void handle_newCountry();
	
	void handle_editAscent	(const QModelIndex& index);
	void handle_editPeak	(const QModelIndex& index);
	void handle_editTrip	(const QModelIndex& index);
	void handle_editHiker	(const QModelIndex& index);
	void handle_editRegion	(const QModelIndex& index);
	void handle_editRange	(const QModelIndex& index);
	void handle_editCountry	(const QModelIndex& index);
};



#endif // MAIN_WINDOW_H
