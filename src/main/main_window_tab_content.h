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
 * @file main_window_tab_content.h
 * 
 * This file declares the MainWindowTabContent class.
 */

#ifndef MAIN_WINDOW_TAB_CONTENT_H
#define MAIN_WINDOW_TAB_CONTENT_H

#include "src/data/item_types.h"
#include "src/main/inverted_sort_header_view.h"
#include "ui_main_window_tab.h"

#include <QWidget>
#include <QShortcut>



class MainWindowTabContent : public QWidget, private Ui_MainWindowTabContent
{
	Q_OBJECT
	
	MainWindow*				mainWindow;
	const ItemTypesHandler*	typesHandler;
	ItemTypeMapper*			mapper;
	Database*				db;
	CompositeTable*			compTable;
	
	bool isViewable;
	bool isDuplicable;
	
private:
	/** The custom horizontal header view for the UI table. */
	InvertedSortHeaderView* headerView;
	
	/** The context menu for the column header area of the UI table. */
	QMenu columnContextMenu;
	/** The column context menu entry for hiding the selected column. */
	QAction* columnContextMenuHideColumnAction;
	/** The column context menu entry for removing the selected column. */
	QAction* columnContextMenuRemoveColumnAction;
	/** The column context submenu for unhiding any previously hidden column. */
	QMenu* columnContextMenuRestoreColumnMenu;
	/** The column context menu entry for creating a new custom column. */
	QAction* columnContextMenuAddCustomColumnAction;
	
	/** The context menu for the cells area of the UI table. */
	QMenu tableContextMenu;
	/** The context menu entry for opening the selected item. */
	QAction* tableContextMenuOpenAction;
	/** The context menu entry for editing the selected item. */
	QAction* tableContextMenuEditAction;
	/** The context menu entry for duplicating the selected item. */
	QAction* tableContextMenuDuplicateAction;
	/** The context menu entries and their target item types for editing directly referenced items. */
	QList<QPair<const ItemTypeMapper*, QAction*>> tableContextMenuEditOtherActions;
	/** The context menu entry for deleting the selected items. */
	QAction* tableContextMenuDeleteAction;
	
	/** List of keyboard shortcuts. */
	QList<QShortcut*> shortcuts;
	
public:
	MainWindowTabContent(QWidget* parent = nullptr);
	~MainWindowTabContent();
	
	// Initial setup
	void init(MainWindow* mainWindow, const ItemTypesHandler* typesHandler, ItemTypeMapper* mapper, Database& db, bool viewable, bool duplicable);
	void restoreColumnWidths();
	void restoreColumnOrder();
	void restoreColumnHiddenStatus();
	void setSorting();
	void initColumnContextMenu();
	void initTableContextMenuAndShortcuts();
	
public:
	QTableView*		getTableView();
	FilterBar*		getFilterBar();
	QScrollArea*	getStatsScrollArea();
	
	void openColumnWizard();
	void refreshStats();
	
	QPair<QSet<BufferRowIndex>, BufferRowIndex> getSelectedRows() const;
	
private slots:
	// UI event handlers
	void handle_tableSelectionChanged();
	void handle_rightClickOnColumnHeader(QPoint pos);
	void handle_rightClickInTable(QPoint pos);
	
	// Column context menu action handlers
	void handle_hideColumn();
	void handle_unhideColumn();
	void handle_addCustomColumn();
	void handle_columnWizardAccepted();
	void handle_removeCustomColumn();
	
private:
	// Helpers
	QList<PALItemType> getDirectlyReferencedTypes() const;
};



#endif // MAIN_WINDOW_TAB_CONTENT_H
