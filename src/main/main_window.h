/*
 * Copyright 2023 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "src/comp_tables/filter.h"
#include "ui_main_window.h"
#include "src/db/database.h"
#include "src/main/item_types_handler.h"

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
	QAction* tableContextMenuEditAction;
	QAction* tableContextMenuDuplicateAction;
	QList<QShortcut*> shortcuts;
	QLabel* statusBarTableSizeLabel;
	QLabel* statusBarFiltersLabel;
	
	const ItemTypesHandler* typesHandler;
	
	// Debugging table views
	bool showDebugTableViews;
	QTableView* photosDebugTableView;
	QTableView* participatedDebugTableView;
	
public:
	MainWindow();
	~MainWindow();
	
private:
    // Initial setup
    void setupMenuIcons();
	void createTypesHandler();
	void connectUI();
	void setupTableViews();
	void setupDebugTableViews();
	void restoreColumnWidths(const ItemTypeMapper& mapper);
	void setSorting(const ItemTypeMapper& mapper);
	void initTableContextMenuAndShortcuts();
	void updateContextMenuEditIcon();
	
	// Project setup (on load)
	void initCompositeBuffers();
	void insertFiltersIntoUI(QSet<Filter> filters);
	
	// UI updates
	void setUIEnabled(bool enabled);
	void updateRecentFilesMenu();
public:
	void updateTableSize(bool reset = false);
	
private:
	// Execute user commands
	void viewItem(const ItemTypeMapper& mapper, int viewRowIndex);
	void newItem(const ItemTypeMapper& mapper);
	void duplicateAndEditItem(const ItemTypeMapper& mapper, int viewRowIndex);
	void editItem(const ItemTypeMapper& mapper, const QModelIndex& index);
	void deleteItem(const ItemTypeMapper& mapper, int viewRowIndex);
	// Helpers
	void performUpdatesAfterUserAction(const ItemTypeMapper& mapper, bool numberOfEntriesChanged, int bufferRowToSelectIndex = -1);
	void updateFilters(const ItemTypeMapper* mapper = nullptr);
public:
	void updateSelectionAfterUserAction(const ItemTypeMapper& mapper, int viewRowIndex);
	
private slots:
	// UI event handlers
	void handle_tabChanged();
	void handle_rightClick(QPoint pos);
	
	// Context menu action handlers
	void handle_viewSelectedItem();
	void handle_editSelectedItem();
	void handle_duplicateAndEditSelectedItem();
	void handle_deleteSelectedItem();
	
	// File menu action handlers
	void handle_newDatabase();
	void handle_openDatabase();
	void handle_openRecentDatabase(QString filepath);
	void handle_clearRecentDatabasesList();
	void handle_saveDatabaseAs();
	void handle_closeDatabase();
	void handle_openProjectSettings();
	void handle_openSettings();
	// View menu action handlers
	void handle_showFiltersChanged();
	void handle_autoResizeColumns();
	// Tools menu action handlers
	void handle_relocatePhotos();
	// Help menu action handlers
	void handle_about();
	
private:
	// Closing behaviour
	void closeEvent(QCloseEvent* event) override;
	void saveImplicitSettings() const;
	void saveColumnWidths(const ItemTypeMapper& mapper) const;
	void saveSorting(const ItemTypeMapper& mapper) const;
	
	// General helpers
	QTableView* getCurrentTableView() const;
	void addToRecentFilesList(const QString& filepath);
	void setWindowTitleFilename(QString filepath = QString());
	void setStatusLine(QString content);
};



#endif // MAIN_WINDOW_H
