#ifndef ASCENT_FILTER_BAR_H
#define ASCENT_FILTER_BAR_H

#include "src/comp_tables/filter.h"
#include "ui_ascent_filter_bar.h"
#include "src/main/item_types_handler.h"

#include <QMainWindow>
#include <QTableView>

class MainWindow;



class AscentFilterBar : public QWidget, public Ui_AscentFilterBar
{
	Q_OBJECT
	
	MainWindow* mainWindow;
	Database* db;
	CompositeAscentsTable* compAscents;
	
public:
	AscentFilterBar(QWidget* parent);
	
	void supplyPointers(MainWindow* mainWindow, Database* db, CompositeAscentsTable* compAscents);
	
private:
	// Initial setup
	void connectUI();
	void setupUI();
	void updateUI();
public:
	void insertFiltersIntoUI(QSet<Filter> filters);
	void resetUI();
	
private:
	// Execute user commands
	void newItem(const ItemTypeMapper& mapper);
	void duplicateAndEditItem(const ItemTypeMapper& mapper, int viewRowIndex);
	void editItem(const ItemTypeMapper& mapper, const QModelIndex& index);
	void deleteItem(const ItemTypeMapper& mapper, int viewRowIndex);
	void updateSelectionAfterUserAction(const ItemTypeMapper& mapper, int viewRowIndex);
	
private slots:
	// Filter event handlers
	void handle_filtersChanged();
	void handle_difficultyFilterBoxChanged();
	void handle_difficultyFilterSystemChanged();
	void handle_applyFilters();
public:
	void handle_clearFilters();
	
private:
	// General helpers
	QSet<Filter> collectAndSaveFilters();
	void clearSavedFilters();
};



#endif // ASCENT_FILTER_BAR_H
