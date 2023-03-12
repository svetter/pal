#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ui_main_window.h"
#include "src/db/database.h"
#include "src/main/settings.h"
#include "src/comp_tables/comp_ascents_table.h"
#include "src/comp_tables/comp_peaks_table.h"
#include "src/comp_tables/comp_trips_table.h"
#include "src/comp_tables/comp_hikers_table.h"
#include "src/comp_tables/comp_regions_table.h"
#include "src/comp_tables/comp_ranges_table.h"
#include "src/comp_tables/comp_countries_table.h"

#include <QMainWindow>
#include <QTableView>
#include <QShortcut>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow, public Ui_MainWindow
{
	Q_OBJECT
	
	Database db;
	QList<QAction*> openRecentActions;
	QMenu tableContextMenu;
	QAction* tableContextMenuOpenAction;
	QAction* tableContextMenuDuplicateAction;
	QList<QShortcut*> shortcuts;
	
	CompositeAscentsTable	compAscents;
	CompositePeaksTable		compPeaks;
	CompositeTripsTable		compTrips;
	CompositeHikersTable	compHikers;
	CompositeRegionsTable	compRegions;
	CompositeRangesTable	compRanges;
	CompositeCountriesTable	compCountries;
	QList<CompositeTable*>	compTables;
	
public:
	MainWindow();
	~MainWindow();
	
	void setStatusLine(QString content);
	void updateAscentCounter();
	
private:
	void setUIEnabled(bool enabled);
	void updateRecentFilesMenu();
	void clearRecentFilesMenu();
	void initCompositeBuffers();
	void setupTableView(QTableView* view, CompositeTable* table, const Setting<QStringList>* columnWidthsSetting);
	void setupDebugTableView(QTableView* view, Table* table);
	void setColumnWidths(QTableView* view, const CompositeTable* table, const Setting<QStringList>* columnWidthsSetting);
	void initTableContextMenuAndShortcuts();
	
private slots:
	void handle_openSelectedItem();
	void handle_editSelectedItem();
	void handle_duplicateAndEditSelectedItem();
	void handle_deleteSelectedItem();
	
private:
	void handle_newItem(int (*openNewItemDialogAndStoreMethod) (QWidget*, Database*), CompositeTable* table, QTableView* tableView);
	void handle_duplicateAndEditItem(int (*openDuplicateItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, QTableView* tableView, int viewRowIndex);
	void handle_editItem(void (*openEditItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, QTableView* tableView, const QModelIndex& index);
	void handle_deleteItem(void (*openDeleteItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, int viewRowIndex);
	void updateSelectionAfterUserAction(QTableView* tableView, CompositeTable* compTable, int viewRowIndex);
	
private slots:
	void handle_openAscent(const QModelIndex& index);
	
	void handle_rightClick(QPoint pos);
	
	void handle_newDatabase();
	void handle_openDatabase();
	void handle_openRecentDatabase(QString filepath);
	void handle_clearRecentDatabasesList();
	void handle_saveDatabaseAs();
	void handle_closeDatabase();
	
	void handle_openProjectSettings();
	void handle_openSettings();
	
	void handle_relocatePhotos();
	
	void handle_about();
	
private:
	void closeEvent(QCloseEvent* event) override;
	void saveImplicitSettings() const;	
	void saveColumnWidths(QTableView* view, const CompositeTable* table, const Setting<QStringList>* columnWidthsSetting) const;
	void addToRecentFilesList(const QString& filepath);
	
	QTableView* getCurrentTableView() const;
	
	
	
private slots:
	void handle_newAscent();
	void handle_newPeak();
	void handle_newTrip();
	void handle_newHiker();
	void handle_newRegion();
	void handle_newRange();
	void handle_newCountry();
	
	void handle_duplicateAndEditAscent	(int viewRowIndex);
	void handle_duplicateAndEditPeak	(int viewRowIndex);
	
	void handle_editAscent	(const QModelIndex& index);
	void handle_editPeak	(const QModelIndex& index);
	void handle_editTrip	(const QModelIndex& index);
	void handle_editHiker	(const QModelIndex& index);
	void handle_editRegion	(const QModelIndex& index);
	void handle_editRange	(const QModelIndex& index);
	void handle_editCountry	(const QModelIndex& index);
	
	void handle_deleteAscent	(int viewRowIndex);
	void handle_deletePeak		(int viewRowIndex);
	void handle_deleteTrip		(int viewRowIndex);
	void handle_deleteHiker		(int viewRowIndex);
	void handle_deleteRegion	(int viewRowIndex);
	void handle_deleteRange		(int viewRowIndex);
	void handle_deleteCountry	(int viewRowIndex);
};



#endif // MAIN_WINDOW_H
