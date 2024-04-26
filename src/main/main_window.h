/*
 * Copyright 2023-2024 Simon Vetter
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

/**
 * @file main_window.h
 * 
 * This file declares the MainWindow class.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ui_main_window.h"
#include "src/db/database.h"
#include "src/main/item_types_handler.h"

#include <QMainWindow>
#include <QTableView>
#include <QShortcut>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



/**
 * Control class for the main window of the application.
 */
class MainWindow : public QMainWindow, private Ui_MainWindow
{
	Q_OBJECT
	
	/** The duration of temporary messages in the status bar in seconds. */
	static const int DYNAMIC_STATUS_MESSAGE_DURATION_SEC = 10;
	
	bool projectOpen;
	/** The project database. */
	Database db;
	/** The ItemTypesHandler singleton. */
	const ItemTypesHandler* typesHandler;
	
	/** The list of menu items for opening the most recently opened database files. */
	QList<QAction*> openRecentActions;
	
	/** The context menu for the column header area of all UI tables. */
	QMenu columnContextMenu;
	/** The column context menu entry for hiding the selected column. */
	QAction* columnContextMenuHideColumnAction;
	/** The column context menu entry for removing the selected column. */
	QAction* columnContextMenuRemoveColumnAction;
	/** The column context submenu for unhiding any previously hidden column. */
	QMenu* columnContextMenuRestoreColumnMenu;
	/** The column context menu entry for creating a new custom column. */
	QAction* columnContextMenuAddCustomColumnAction;
	
	/** The context menu for the cell are of all UI tables. */
	QMenu tableContextMenu;
	/** The context menu entry for opening the selected item. */
	QAction* tableContextMenuOpenAction;
	/** The context menu entry for editing the selected item. */
	QAction* tableContextMenuEditAction;
	/** The context menu entry for duplicating the selected item. */
	QAction* tableContextMenuDuplicateAction;
	/** The context menu entry for deleting the selected items. */
	QAction* tableContextMenuDeleteAction;
	
	/** List of keyboard shortcuts. */
	QList<QShortcut*> shortcuts;
	
	/** The status bar label for the current table size. */
	QLabel* statusBarTableSizeLabel;
	/** The status bar label for the current filter settings. */
	QLabel* statusBarFiltersLabel;
	
	/** The stats engine instance for computing general statistics. */
	GeneralStatsEngine generalStatsEngine;
	
public:
	MainWindow();
	~MainWindow();
	
private:
	// Initial setup
	void createTypesHandler();
	void setupMenuIcons();
	void connectUI();
	void setupTableTabs();
	void restoreColumnWidths(const ItemTypeMapper* const mapper);
	void restoreColumnOrder(const ItemTypeMapper* const mapper);
	void restoreColumnHiddenStatus(const ItemTypeMapper* const mapper);
	void setSorting(const ItemTypeMapper* const mapper);
	void initColumnContextMenu();
	void initTableContextMenuAndShortcuts();
	void updateTableContextMenuIcons();
	
	// Project setup (on load)
	void attemptToOpenFile(const QString& filepath);
	void initCompositeBuffers();
	
	// UI updates
	void setUIEnabled(bool enabled);
	void updateRecentFilesMenu();
	void updateItemCountDisplays(bool reset = false);
	
public:
	void currentFiltersChanged();
	
private:
	// Execute user commands
	void viewSelectedItem();
	void newItem(const ItemTypeMapper& mapper);
	void duplicateAndEditSelectedItem();
	void editSelectedItems();
	void deleteSelectedItems();
	// Helpers
	void setFilteredAscentsCounterVisible(bool visible);
	void updateTopBarButtonVisibilities();
	void performUpdatesAfterUserAction(const ItemTypeMapper& mapper, bool numberOfEntriesChanged, BufferRowIndex bufferRowToSelectIndex = BufferRowIndex());
	void scrollToTopAfterSorting();
	void updateFilters(const ItemTypeMapper* onlyForMapper = nullptr);
public:
	void updateSelectionAfterUserAction(const ItemTypeMapper& mapper, ViewRowIndex viewRowIndex);
	
private slots:
	// UI event handlers
	void handle_tabChanged();
	void handle_tableSelectionChanged();
	void handle_rightClickOnColumnHeader(QPoint pos);
	void handle_rightClickInTable(QPoint pos);
	
	// Column context menu action handlers
	void handle_hideColumn();
	void handle_unhideColumn();
	void handle_addCustomColumn();
	void handle_columnWizardAccepted();
	void handle_removeCustomColumn();
	
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
	void handle_showStatsPanelChanged();
	void handle_showAllStatsPanels();
	void handle_hideAllStatsPanels();
	void handle_chartRangesPinnedChanged();
	void handle_autoResizeColumns();
	void handle_resetColumnOrder();
	void handle_restoreHiddenColumns();
	void handle_clearTableSelection();
	// Tools menu action handlers
	void handle_relocatePhotos();
	void handle_exportData();
	// Help menu action handlers
	void handle_about();
	
private:
	// Closing behaviour
	void closeEvent(QCloseEvent* event) override;
	void saveProjectImplicitSettings();
	void saveGlobalImplicitSettings();
	void saveImplicitColumnSettings(const ItemTypeMapper& mapper);
	void saveSorting(const ItemTypeMapper& mapper);
	// Layout changes
	virtual void resizeEvent(QResizeEvent* event) override;
	
	// General helpers
	ItemTypeMapper* getActiveMapperOrNull() const;
	ItemTypeMapper& getActiveMapper() const;
	QPair<QSet<BufferRowIndex>, BufferRowIndex> getSelectedRows(const ItemTypeMapper& mapper) const;
	void addToRecentFilesList(const QString& filepath);
	void setWindowTitleFilename(QString filepath = QString());
	void setStatusLine(QString content);
};



#endif // MAIN_WINDOW_H
