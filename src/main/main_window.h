#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "src/main/item_types_handler.h"
#include "ui_main_window.h"
#include "src/db/database.h"
#include "src/main/settings.h"

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
	
	const ItemTypesHandler* typesHandler;
	
public:
	MainWindow();
	~MainWindow();
	
private:
	// Initial setup
	void connectUI();
	void setupTableViews();
	void setupDebugTableViews();
	void setColumnWidths(QTableView* view, const CompositeTable* table, const Setting<QStringList>* columnWidthsSetting);
	void initTableContextMenuAndShortcuts();
	
	// Project setup (on load)
	void initCompositeBuffers();
	
	// UI updates
	void setUIEnabled(bool enabled);
	void updateRecentFilesMenu();
	void updateAscentCounter();
	
	// Execute user commands
	void newItem(int (*openNewItemDialogAndStoreMethod) (QWidget*, Database*), CompositeTable* table, QTableView* tableView);
	void duplicateAndEditItem(int (*openDuplicateItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, QTableView* tableView, int viewRowIndex);
	void editItem(void (*openEditItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, QTableView* tableView, const QModelIndex& index);
	void deleteItem(void (*openDeleteItemDialogAndStoreMethod) (QWidget*, Database*, int), CompositeTable* compTable, int viewRowIndex);
	void updateSelectionAfterUserAction(QTableView* tableView, CompositeTable* compTable, int viewRowIndex);
	
private slots:
	// UI event handlers
	void handle_tabChanged();
	void handle_rightClick(QPoint pos);
	
	// Context menu action handlers
	void handle_openSelectedItem();
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
	// Tools menu action handlers
	void handle_relocatePhotos();
	// Help menu action handlers
	void handle_about();
	
private:
	// Closing behaviour
	void closeEvent(QCloseEvent* event) override;
	void saveImplicitSettings() const;
	
	// General helpers
	QTableView* getCurrentTableView() const;
	void addToRecentFilesList(const QString& filepath);
	
public:
	// Database callback
	void setStatusLine(QString content);
};



#endif // MAIN_WINDOW_H
