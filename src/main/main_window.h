#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "src/db/database.h"
#include "src/main/settings.h"
#include "ui_main_window.h"

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
	
public:
	MainWindow();
	~MainWindow();
	
	void setStatusLine(QString content);
	void updateAscentCounter();
	
private:
	void setUIEnabled(bool enabled);
	void updateRecentFilesMenu();
	void clearRecentFilesMenu();
	void setupTableView(QTableView* view, NormalTable* table, const Setting<QStringList>* columnWidthsSetting);
	void setColumnWidths(QTableView* view, NormalTable* table, const Setting<QStringList>* columnWidthsSetting);
	void initTableContextMenuAndShortcuts();
	
private slots:
	void handle_openSelectedItem();
	void handle_editSelectedItem();
	void handle_duplicateAndEditSelectedItem();
	void handle_deleteSelectedItem();
	
	void handle_newAscent();
	void handle_newPeak();
	void handle_newTrip();
	void handle_newHiker();
	void handle_newRegion();
	void handle_newRange();
	void handle_newCountry();
	
	void handle_openAscent	(const QModelIndex& index);
	
	void handle_editAscent	(const QModelIndex& index);
	void handle_editPeak	(const QModelIndex& index);
	void handle_editTrip	(const QModelIndex& index);
	void handle_editHiker	(const QModelIndex& index);
	void handle_editRegion	(const QModelIndex& index);
	void handle_editRange	(const QModelIndex& index);
	void handle_editCountry	(const QModelIndex& index);
	
	void handle_duplicateAndEditAscent	(int rowIndex);
	void handle_duplicateAndEditPeak	(int rowIndex);
	
	void handle_deleteAscent	(int rowIndex);
	void handle_deletePeak		(int rowIndex);
	void handle_deleteTrip		(int rowIndex);
	void handle_deleteHiker		(int rowIndex);
	void handle_deleteRegion	(int rowIndex);
	void handle_deleteRange		(int rowIndex);
	void handle_deleteCountry	(int rowIndex);
	
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
	void addToRecentFilesList(const QString& filepath);
	
	QTableView* getCurrentTableView() const;
	void handle_newItem(int (*openNewItemDialogAndStoreMethod) (QWidget*, Database*), NormalTable* table, QTableView* view);
	void saveColumnWidths(QTableView* view, NormalTable* table, const Setting<QStringList>* columnWidthsSetting) const;
};



#endif // MAIN_WINDOW_H
