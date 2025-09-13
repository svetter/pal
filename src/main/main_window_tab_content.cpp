/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file main_window_tab_content.cpp
 * 
 * This file defines the MainWindowTabContent class.
 */

#include "main_window_tab_content.h"

#include "main_window.h"



MainWindowTabContent::MainWindowTabContent(QWidget* parent) :
	QWidget(parent),
	mainWindow(nullptr),
	typesHandler(nullptr),
	mapper(nullptr),
	db(nullptr),
	compTable(nullptr),
	isViewable(false),
	isDuplicable(false),
	headerView(nullptr),
	columnContextMenu(QMenu(this)),
	columnContextMenuHideColumnAction(nullptr),
	columnContextMenuRestoreColumnMenu(nullptr),
	tableContextMenu(QMenu(this)),
	tableContextMenuOpenAction(nullptr),
	tableContextMenuDuplicateAction(nullptr),
	tableContextMenuEditOtherActions(QList<QPair<const ItemTypeMapper*, QAction*>>()),
	shortcuts(QList<QShortcut*>())
{
	setupUi(this);
}

MainWindowTabContent::~MainWindowTabContent()
{
	delete headerView;
	qDeleteAll(shortcuts);
	delete columnContextMenuRestoreColumnMenu;
}



void MainWindowTabContent::init(MainWindow* mainWindow, const ItemTypesHandler* typesHandler, ItemTypeMapper* mapper, Database& db, bool viewable, bool duplicable)
{
	assert(mainWindow && mapper);
	
	this->mainWindow	= mainWindow;
	this->typesHandler	= typesHandler;
	this->mapper		= mapper;
	this->db			= &db;
	this->compTable		= &mapper->compTable;
	this->isViewable	= viewable;
	this->isDuplicable	= duplicable;
	
	
	// Set horizontal header
	this->headerView = new InvertedSortHeaderView(tableView, *compTable);
	headerView->setSectionsClickable(true);
	tableView->setHorizontalHeader(headerView);
	
	// Set model
	tableView->setModel(compTable);
	compTable->setUpdateImmediately(mapper->type == mainWindow->getCurrentTabIndex());
	
	// Enable column header reordering
	tableView->horizontalHeader()->setSectionsMovable(true);
	
	// Create context menus
	initColumnContextMenu();
	initTableContextMenuAndShortcuts();
	// Connect context menus
	tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MainWindowTabContent::handle_rightClickOnColumnHeader);
	connect(tableView, &QTableView::customContextMenuRequested, this, &MainWindowTabContent::handle_rightClickInTable);
	
	// Connect selection change listener
	connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindowTabContent::handle_tableSelectionChanged);
	
	// Stats visibility
	const bool statsEnabled = mapper->showStatsPanelSetting.get();
	statsScrollArea->setVisible(statsEnabled);
	// Restore splitter sizes
	QSplitter* const splitter = mapper->tab.findChild<QSplitter*>();
	splitter->setStretchFactor(0, 3);
	splitter->setStretchFactor(1, 1);
	restoreSplitterSizes(*splitter, mapper->statsPanelSplitterSizesSetting);
	
	// Setup stats panels
	mapper->statsEngine.setupStatsPanel();
	mapper->statsEngine.setCurrentlyVisible(false);
	mapper->statsEngine.setRangesPinned(mainWindow->getPinStatRangesState());
	
	// Connect column wizard
	connect(&mapper->columnWizard, &ColumnWizard::accepted, this, &MainWindowTabContent::handle_columnWizardAccepted);
}


/**
 * Restores the column widths for the table view.
 */
void MainWindowTabContent::restoreColumnWidths()
{
	QSet<QString> columnNameSet = compTable->getNormalColumnNameSet();
	if (mapper->columnWidthsSetting.nonePresent(columnNameSet)) return;	// Only restore if any widths are in the settings
	
	const QSet<QString> normalColumnNames = compTable->getNormalColumnNameSet();
	const QMap<QString, int> columnWidthMap = mapper->columnWidthsSetting.get(normalColumnNames);
	
	// Restore column widths
	for (int columnIndex = 0; columnIndex < compTable->getNumberOfNormalColumns(); columnIndex++) {
		const QString& columnName = compTable->getColumnAt(columnIndex).name;
		int columnWidth = columnWidthMap[columnName];
		if (columnWidth < 1) {
			columnWidth = tableView->horizontalHeader()->sizeHintForColumn(columnIndex);
		}
		tableView->setColumnWidth(columnIndex, columnWidth);
	}
}

/**
 * Restores the column order for the table view.
 */
void MainWindowTabContent::restoreColumnOrder()
{
	QSet<QString> columnNameSet = compTable->getNormalColumnNameSet();
	if (mapper->columnOrderSetting.nonePresent(columnNameSet)) return;	// Only restore if any columns are in the settings
	
	const QSet<QString> normalColumnNames = compTable->getNormalColumnNameSet();
	const QMap<QString, int> columnOrderMap = mapper->columnOrderSetting.get(normalColumnNames);
	// Sort by visual index
	QList<QPair<const CompositeColumn*, int>> columnOrderList = QList<QPair<const CompositeColumn*, int>>();
	for (const QPair<QString, int>& columnOrderPair : columnOrderMap.asKeyValueRange()) {
		if (columnOrderPair.second < 0) continue;	// Visual index invalid, ignore column
		const CompositeColumn* column = compTable->getColumnByNameOrNull(columnOrderPair.first);
		if (!column) continue;
		columnOrderList.append({column, columnOrderPair.second});
	}
	auto comparator = [](const QPair<const CompositeColumn*, int>& pair1, const QPair<const CompositeColumn*, int>& pair2) {
		return pair1.second < pair2.second;
	};
	std::sort(columnOrderList.begin(), columnOrderList.end(), comparator);
	
	// Restore column order
	QHeaderView* header = tableView->horizontalHeader();
	for (int visualIndex = 0; visualIndex < columnOrderList.size(); visualIndex++) {
		const CompositeColumn& column = *columnOrderList.at(visualIndex).first;
		int logicalIndex = column.getIndex();
		int currentVisualIndex = header->visualIndex(logicalIndex);
		header->moveSection(currentVisualIndex, visualIndex);
	}
}

/**
 * Restores the column hidden states for the table view.
 */
void MainWindowTabContent::restoreColumnHiddenStatus()
{
	QSet<QString> columnNameSet = compTable->getNormalColumnNameSet();
	if (mapper->hiddenColumnsSetting.nonePresent(columnNameSet)) return;	// Only restore if any column are in the settings
	
	const QSet<QString> normalColumnNames = compTable->getNormalColumnNameSet();
	const QMap<QString, bool> columnHiddenMap = mapper->hiddenColumnsSetting.get(normalColumnNames);
	
	// Restore column hidden status
	for (int columnIndex = 0; columnIndex < compTable->getNumberOfNormalColumns(); columnIndex++) {
		const QString& columnName = compTable->getColumnAt(columnIndex).name;
		bool storedColumnHiddenStatus = columnHiddenMap[columnName];
		if (!storedColumnHiddenStatus) continue;
		tableView->horizontalHeader()->setSectionHidden(columnIndex, true);
		compTable->markColumnHidden(columnIndex);
	}
}

/**
 * Sets the sorting for the table view to either the remembered sorting or, if that is not present
 * or disabled, to the default sorting.
 */
void MainWindowTabContent::setSorting()
{
	SortingPass sorting = compTable->getDefaultSorting();
	bool sortingSettingValid = true;
	
	while (Settings::rememberSorting.get() && mapper->sortingSetting.present()) {
		sortingSettingValid = false;
		
		QStringList saved = mapper->sortingSetting.get().split(",");
		if (saved.size() != 2) break;
		
		const CompositeColumn* column = compTable->getColumnByNameOrNull(saved.at(0).trimmed());
		if (!column) break;
		
		bool ascending = saved.at(1).trimmed().compare("Descending", Qt::CaseInsensitive) != 0;
		Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
		
		sorting.column = column;
		sorting.order = order;
		sortingSettingValid = true;
		break;
	}
	tableView->sortByColumn(sorting.column->getIndex(), sorting.order);
	
	if (!sortingSettingValid) mapper->sortingSetting.clear(*this);
}



/**
 * Initializes the column context menu.
 */
void MainWindowTabContent::initColumnContextMenu()
{
	// Context menu
	columnContextMenuHideColumnAction = columnContextMenu.addAction(tr("Hide this column"));
	columnContextMenuRemoveColumnAction = columnContextMenu.addAction(tr("Remove this column"));
	columnContextMenuRemoveColumnAction->setEnabled(false);
	columnContextMenu.addSeparator();
	columnContextMenuRestoreColumnMenu = columnContextMenu.addMenu(tr("Restore hidden column"));
	columnContextMenu.addSeparator();
	columnContextMenuAddCustomColumnAction = columnContextMenu.addAction(tr("Add custom column..."));
	
	// Set icons
	columnContextMenuHideColumnAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	columnContextMenuRemoveColumnAction		->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	columnContextMenuAddCustomColumnAction	->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
	
	connect(columnContextMenuHideColumnAction,		&QAction::triggered, this, &MainWindowTabContent::handle_hideColumn);
	connect(columnContextMenuRemoveColumnAction,	&QAction::triggered, this, &MainWindowTabContent::handle_removeCustomColumn);
	connect(columnContextMenuAddCustomColumnAction,	&QAction::triggered, this, &MainWindowTabContent::handle_addCustomColumn);
}

/**
 * Initializes the table context menu and the keyboard shortcuts for the table views.
 */
void MainWindowTabContent::initTableContextMenuAndShortcuts()
{
	QKeySequence openKeySequence		= QKeySequence(Qt::Key_Return);
	QKeySequence editKeySequence		= QKeySequence(Qt::CTRL | Qt::Key_Return);
	QKeySequence duplicateKeySequence	= QKeySequence::Copy;
	QKeySequence deleteKeySequence		= QKeySequence::Delete;
	
	const QList<PALItemType> directlyReferencedTypes = getDirectlyReferencedTypes();
	
	// Context menu
	tableContextMenuOpenAction = tableContextMenu.addAction(tr("View..."), openKeySequence);
	
	tableContextMenu.addSeparator();
	tableContextMenuEditAction = tableContextMenu.addAction(tr("Edit..."), editKeySequence);
	if (isDuplicable) {
		tableContextMenuDuplicateAction	= tableContextMenu.addAction(tr("Edit as new duplicate..."), duplicateKeySequence);
	}
	
	if (!directlyReferencedTypes.isEmpty()) {
		tableContextMenu.addSeparator();
		// Create actions for editing directly (forward only) referenced item types
		for (const PALItemType& directlyReferencedType : directlyReferencedTypes) {
			const ItemTypeMapper& targetMapper = typesHandler->get(directlyReferencedType);
			QAction* const editOtherAction = tableContextMenu.addAction(targetMapper.baseTable.getEditItemString());
			editOtherAction->setData(QVariant(directlyReferencedType));
			tableContextMenuEditOtherActions.append({&targetMapper, editOtherAction});
		}
	}
	
	tableContextMenu.addSeparator();
	tableContextMenuDeleteAction = tableContextMenu.addAction(tr("Delete"), deleteKeySequence);
	
	// Set icons
	const QIcon icon = QIcon(":/icons/" + mapper->name + ".svg");
	tableContextMenuEditAction->setIcon(icon);
	if (isDuplicable) tableContextMenuDuplicateAction->setIcon(icon);
	for (const auto& [otherMapper, editOtherAction] : std::as_const(tableContextMenuEditOtherActions)) {
		const QIcon otherIcon = QIcon(":/icons/" + otherMapper->name + ".svg");
		editOtherAction->setIcon(otherIcon);
	}
	tableContextMenuOpenAction->setIcon(QIcon(":/icons/ascent_viewer.svg"));
	tableContextMenuDeleteAction->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	
	// Connect actions
	connect(tableContextMenuOpenAction,				&QAction::triggered, mainWindow, &MainWindow::viewSelectedItem);
	connect(tableContextMenuEditAction,				&QAction::triggered, mainWindow, &MainWindow::editSelectedItems);
	if (isDuplicable) {
		connect(tableContextMenuDuplicateAction,	&QAction::triggered, mainWindow, &MainWindow::duplicateAndEditSelectedItem);
	}
	for (const auto& [_, editOtherAction] : std::as_const(tableContextMenuEditOtherActions)) {
		connect(editOtherAction,					&QAction::triggered, mainWindow, &MainWindow::editSelectedItemReferenced);
	}
	connect(tableContextMenuDeleteAction,			&QAction::triggered, mainWindow, &MainWindow::deleteSelectedItems);
	
	
	// Keyboard shortcuts
	QShortcut* openShortcut			= new QShortcut(openKeySequence,		tableView);
	QShortcut* editShortcut			= new QShortcut(editKeySequence,		tableView);
	QShortcut* duplicateShortcut	= new QShortcut(duplicateKeySequence,	tableView);
	QShortcut* deleteShortcut		= new QShortcut(deleteKeySequence,		tableView);
	
	shortcuts.append(openShortcut);
	shortcuts.append(editShortcut);
	shortcuts.append(duplicateShortcut);
	shortcuts.append(deleteShortcut);
	
	auto handle_enter = [this]() {
		if (isViewable)	return mainWindow->viewSelectedItem();
		else			return mainWindow->editSelectedItems();
	};
	connect(openShortcut,		&QShortcut::activated, this,		handle_enter);
	connect(editShortcut,		&QShortcut::activated, mainWindow,	&MainWindow::editSelectedItems);
	connect(duplicateShortcut,	&QShortcut::activated, mainWindow,	&MainWindow::duplicateAndEditSelectedItem);
	connect(deleteShortcut,		&QShortcut::activated, mainWindow,	&MainWindow::deleteSelectedItems);
}



QTableView* MainWindowTabContent::getTableView()
{
	return tableView;
}

FilterBar* MainWindowTabContent::getFilterBar()
{
	return filterBar;
}

QScrollArea* MainWindowTabContent::getStatsScrollArea()
{
	return statsScrollArea;
}



void MainWindowTabContent::openColumnWizard()
{
	handle_addCustomColumn();
}

void MainWindowTabContent::refreshStats()
{
	handle_tableSelectionChanged();
}



/**
 * Returns the indices of selected rows and marked row in the table.
 * 
 * Marked rows which are not selected are ignored and replaced with the buffer row which corresponds
 * with the lowest view row index of the selected rows.
 * 
 * @return	The selected rows in the currently active table.
 */
QPair<QSet<BufferRowIndex>, BufferRowIndex> MainWindowTabContent::getSelectedRows() const
{
	const QModelIndex markedModelIndex = tableView->currentIndex();
	const QModelIndexList selectedModelIndices = tableView->selectionModel()->selectedRows();
	
	if (selectedModelIndices.isEmpty()) {
		if (!markedModelIndex.isValid()) return {{}, BufferRowIndex()};
		
		const ViewRowIndex markedViewRow = ViewRowIndex(markedModelIndex.row());
		const BufferRowIndex marked = compTable->getBufferRowIndexForViewRow(markedViewRow);
		return {{ marked }, marked};
	}
	
	QSet<BufferRowIndex> selected = QSet<BufferRowIndex>();
	for (const QModelIndex& modelIndex : selectedModelIndices) {
		const ViewRowIndex viewRowIndex = ViewRowIndex(modelIndex.row());
		const BufferRowIndex bufferRowIndex = compTable->getBufferRowIndexForViewRow(viewRowIndex);
		assert(bufferRowIndex.isValid(mapper->baseTable.getNumberOfRows()));
		selected.insert(bufferRowIndex);
	}
	
	if (markedModelIndex.isValid()) {
		const ViewRowIndex markedViewRow = ViewRowIndex(markedModelIndex.row());
		const BufferRowIndex marked = compTable->getBufferRowIndexForViewRow(markedViewRow);
		
		if (selected.contains(marked)) {
			return {selected, marked};
		}
	}
	auto bufferRowCompare = [this](const BufferRowIndex& index1, const BufferRowIndex& index2) {
		return compTable->findViewRowIndexForBufferRow(index1) < compTable->findViewRowIndexForBufferRow(index2);
	};
	const BufferRowIndex minBufferRow = *std::min_element(selected.constBegin(), selected.constEnd(), bufferRowCompare);
	return {selected, minBufferRow};
}



// UI EVENT HANDLERS

/**
 * Event handler for changes in which rows of the active table view are selected.
 * 
 * Collects selected rows and updates the item statistics panel.
 */
void MainWindowTabContent::handle_tableSelectionChanged()
{
	const bool statsPanelShown = mainWindow->getShowItemStatsPanelState();
	if (!mainWindow->isProjectOpen() || !statsPanelShown) return;
	
	const QItemSelection selection = tableView->selectionModel()->selection();
	QSet<BufferRowIndex> selectedBufferRows = QSet<BufferRowIndex>();
	if (selection.isEmpty()) {
		// Instead of showing an empty chart, show chart for all rows (except filtered out)
		int numRowsShown = compTable->rowCount();
		for (ViewRowIndex viewIndex = ViewRowIndex(0); viewIndex.isValid(numRowsShown); viewIndex++) {
			BufferRowIndex bufferIndex = compTable->getBufferRowIndexForViewRow(viewIndex);
			selectedBufferRows.insert(bufferIndex);
		}
	}
	else {
		// One or more rows selected, find their buffer indices
		QSet<ViewRowIndex> selectedViewRows = QSet<ViewRowIndex>();
		for (const QItemSelectionRange& range : selection) {
			const QModelIndexList rangeIndices = range.indexes();
			for (const QModelIndex& index : rangeIndices) {
				selectedViewRows.insert(ViewRowIndex(index.row()));
			}
		}
		for (const ViewRowIndex& viewIndex : std::as_const(selectedViewRows)) {
			BufferRowIndex bufferIndex = compTable->getBufferRowIndexForViewRow(viewIndex);
			selectedBufferRows.insert(bufferIndex);
		}
	}
	
	const bool allSelected = compTable->rowCount() == selectedBufferRows.size();
	mapper->statsEngine.setStartBufferRows(selectedBufferRows, allSelected);
}

/**
 * Event handler for right clicks on the column header area of the active table view.
 * 
 * Prepares and opens the column context menu at the given position.
 * 
 * @param pos	The position of the right click in the viewport of the horizontal table view header.
 */
void MainWindowTabContent::handle_rightClickOnColumnHeader(QPoint pos)
{
	// Get index of clicked column
	QHeaderView* header = tableView->horizontalHeader();
	int logicalIndexClicked = header->logicalIndexAt(pos);
	if (logicalIndexClicked < 0) return;
	
	// Repopulate 'restore column' submenu
	int visibleColumns = 0;
	columnContextMenuRestoreColumnMenu->clear();
	for (int logicalIndex = 0; logicalIndex < header->count(); logicalIndex++) {
		if (!header->isSectionHidden(logicalIndex)) {
			visibleColumns++;
			continue;
		}
		
		const QString& columnName = compTable->getColumnAt(logicalIndex).uiName;
		QAction* restoreColumnAction = columnContextMenuRestoreColumnMenu->addAction(columnName);
		restoreColumnAction->setData(logicalIndex);
		connect(restoreColumnAction, &QAction::triggered, this, &MainWindowTabContent::handle_unhideColumn);
	}
	columnContextMenuRestoreColumnMenu->setEnabled(!columnContextMenuRestoreColumnMenu->isEmpty());
	
	// Configure 'hide column' and 'remove column' actions
	columnContextMenuHideColumnAction->setData(logicalIndexClicked);
	columnContextMenuRemoveColumnAction->setData(logicalIndexClicked);
	columnContextMenuHideColumnAction->setEnabled(visibleColumns > 1);
	columnContextMenuRemoveColumnAction->setEnabled(compTable->hasCustomColumnAt(logicalIndexClicked));
	
	// Configure 'add custom column' action
	columnContextMenuAddCustomColumnAction->setData(logicalIndexClicked);
	
	// Show context menu
	columnContextMenu.popup(header->viewport()->mapToGlobal(pos));
}

/**
 * Event handler for right clicks in the cell area of the active table view.
 * 
 * Prepares and opens the table context menu at the given position.
 * 
 * @param pos	The position of the right click in the viewport of the table view.
 */
void MainWindowTabContent::handle_rightClickInTable(QPoint pos)
{
	QModelIndex index = tableView->indexAt(pos);
	if (!index.isValid()) return;
	
	const QSet<BufferRowIndex> selectedRows = getSelectedRows().first;
	const bool singleRowSelected = selectedRows.size() == 1;
	const BufferRowIndex singleSelectedRow = singleRowSelected ? BufferRowIndex(selectedRows.constBegin()->get()) : BufferRowIndex();
	tableContextMenuOpenAction->setVisible(singleRowSelected && isViewable);
	if (isDuplicable) tableContextMenuDuplicateAction->setVisible(singleRowSelected);
	
	// Enable or disable referenced item edit actions in table context menu
	for (const auto& [otherMapper, editOtherAction] : std::as_const(tableContextMenuEditOtherActions)) {
		bool enableAction = singleRowSelected;
		if (singleRowSelected) {
			// Check whether reference chain is continuous here
			const Breadcrumbs crumbs = db->getBreadcrumbsFor(mapper->baseTable, otherMapper->baseTable);
			const BufferRowIndex targetBufferRow = crumbs.evaluateAsForwardChain(singleSelectedRow);
			enableAction &= targetBufferRow.isValid();
		}
		
		editOtherAction->setEnabled(enableAction);
	}
	
	QString deleteString = tr("Delete") + (Settings::confirmDelete.get() ? "..." : "");
	tableContextMenuDeleteAction->setText(deleteString);
	
	tableContextMenu.popup(tableView->viewport()->mapToGlobal(pos));
}



// COLUMN CONTEXT MENU ACTION HANDLERS

/**
 * Event handler for the 'hide column' action in the column context menu.
 * 
 * Extracts the column index from calling sender().
 */
void MainWindowTabContent::handle_hideColumn()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;
	
	const int logicalIndex = action->data().toInt();
	
	tableView->horizontalHeader()->setSectionHidden(logicalIndex, true);
	compTable->markColumnHidden(logicalIndex);
}

/**
 * Event handler for any of the 'unhide column' actions in the column context menu.
 * 
 * Extracts the column index from calling sender().
 */
void MainWindowTabContent::handle_unhideColumn()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;
	
	const int logicalIndex = action->data().toInt();
	
	tableView->horizontalHeader()->setSectionHidden(logicalIndex, false);
	compTable->markColumnUnhidden(logicalIndex);
	compTable->updateBothBuffers();
}

/**
 * Event handler for the 'add custom column' action in the column context menu.
 * 
 * Extracts the column index from calling sender().
 */
void MainWindowTabContent::handle_addCustomColumn()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;
	
	const int logicalIndex = action->data().toInt();
	const int visualIndex = tableView->horizontalHeader()->visualIndex(logicalIndex) + 1;
	
	ColumnWizard& wizard = mapper->columnWizard;
	
	wizard.visualIndexToUse = visualIndex;
	wizard.restart();
	wizard.show();
}

void MainWindowTabContent::handle_columnWizardAccepted()
{
	ColumnWizard& wizard = mapper->columnWizard;
	
	const CompositeColumn& newColumn = *wizard.getFinishedColumn();
	compTable->addCustomColumn(newColumn);
	
	// Set visual index to right of the clicked column
	const int logicalIndex = compTable->getIndexOf(newColumn);
	const int currentVisualIndex = tableView->horizontalHeader()->visualIndex(logicalIndex);
	tableView->horizontalHeader()->moveSection(currentVisualIndex, wizard.visualIndexToUse);
}

/**
 * Event handler for the 'remove column' action in the column context menu.
 * 
 * Extracts the column index from calling sender().
 */
void MainWindowTabContent::handle_removeCustomColumn()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;
	
	const int logicalIndex = action->data().toInt();
	
	filterBar->compColumnAboutToBeRemoved(compTable->getColumnAt(logicalIndex));
	compTable->removeCustomColumnAt(logicalIndex);
}



// HELPERS

QList<PALItemType> MainWindowTabContent::getDirectlyReferencedTypes() const
{
	QList<QPair<PALItemType, Breadcrumbs>> forwardOnlyCrumbs = QList<QPair<PALItemType, Breadcrumbs>>();
	
	for (const ItemTypeMapper* const otherMapper : typesHandler->getAllMappers()) {
		if (otherMapper == mapper) continue;
		
		const Breadcrumbs crumbs = db->getBreadcrumbsFor(mapper->baseTable, otherMapper->baseTable);
		if (crumbs.isForwardOnly()) {
			forwardOnlyCrumbs.append({otherMapper->type, crumbs});
		}
	}
	
	std::vector<QPair<PALItemType, Breadcrumbs>> vector = std::vector<QPair<PALItemType, Breadcrumbs>>(forwardOnlyCrumbs.begin(), forwardOnlyCrumbs.end());
	auto comparator = [](const QPair<PALItemType, Breadcrumbs>& p1, const QPair<PALItemType, Breadcrumbs>& p2) {
		return p1.second.length() < p2.second.length();
	};
	std::stable_sort(vector.begin(), vector.end(), comparator);
	
	QList<PALItemType> result = QList<PALItemType>();
	for (const auto& [type, _] : vector) {
		result.append(type);
	}
	return result;
}
