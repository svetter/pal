#ifndef ASCENT_FILTER_BAR_H
#define ASCENT_FILTER_BAR_H

#include "src/comp_tables/filter.h"
#include "src/db/database.h"
#include "ui_ascent_filter_bar.h"

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
	// Initial setup
	void supplyPointers(MainWindow* mainWindow, Database* db, CompositeAscentsTable* compAscents);
private:
	void connectUI();
	void setupUI();
	
public:
	// Project setup
	void resetUI();
	void insertFiltersIntoUI(QSet<Filter> filters);
	
private slots:
	// UI change handlers
	void handle_filtersChanged();
	void handle_difficultyFilterBoxChanged();
	void handle_minDateChanged();
	void handle_maxDateChanged();
	void handle_minHeightChanged();
	void handle_maxHeightChanged();
	void handle_difficultyFilterSystemChanged();
	
public:
	// Execute filter actions
	void handle_applyFilters();
	void handle_clearFilters();
	
private:
	// Parsing filters from UI
	QSet<Filter> collectFilters();
	
	// Saving filters
	void clearSavedFilters();
	void saveFilters(const QSet<Filter> filters);
	
public:
	// Retrieving filters from project settings
	QSet<Filter> parseFiltersFromProjectSettings();
};



#endif // ASCENT_FILTER_BAR_H
