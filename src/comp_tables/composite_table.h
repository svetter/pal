#ifndef COMPOSITE_TABLE_H
#define COMPOSITE_TABLE_H

#include "composite_column.h"
#include "src/db/database.h"

#include <QAbstractTableModel>
#include <QProgressDialog>



class CompositeTable : public QAbstractTableModel {
	const NormalTable* baseTable;
	Database* db;
	
	QList<const CompositeColumn*> columns;
	int firstHiddenColumnIndex;
	
	QList<QList<QVariant>*> buffer;
	QList<int> bufferOrder;
	QPair<const CompositeColumn*, Qt::SortOrder> currentSorting;
	QSet<Filter> currentFilters;
	
	QSet<const CompositeColumn*> columnsToUpdate;
	bool updateImmediately;
	
public:
	const QString name;
	
protected:
	CompositeTable(Database* db, NormalTable* baseTable);
public:
	~CompositeTable();
	
protected:
	void addColumn(const CompositeColumn* column, bool hidden = false);
public:
	const CompositeColumn* getColumnAt(int columnIndex) const;
	int getIndexOf(const CompositeColumn* column) const;
	const NormalTable* getBaseTable() const;
	
	int getNumberOfCellsToInit() const;
	void initBuffer(QProgressDialog* progressDialog, QSet<Filter> filters = QSet<Filter>());
	void rebuildOrderBuffer(bool skipRepopulate = false);
	int getNumberOfCellsToUpdate() const;
	void updateBuffer(QProgressDialog* progressDialog);
	void resetBuffer();
	int getBufferRowForViewRow(int viewRowIndex) const;
	int findCurrentViewRowIndex(int bufferRowIndex) const;
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const = 0;
	QPair<const CompositeColumn*, Qt::SortOrder> getCurrentSorting() const;
	
	void applyFilters(QSet<Filter> filters);
	void clearFilters();
	QSet<Filter> getCurrentFilters() const;
	bool filterIsActive() const;
	
public:
	// Change annunciation
	void setUpdateImmediately(bool updateImmediately, QProgressDialog* progress = nullptr);
	void insertRowAndAnnounce(int bufferRowIndex);
	void removeRowAndAnnounce(int bufferRowIndex);
	void announceChangesUnderColumn(int columnIndex);
	int updateSortingAfterItemEdit(int viewRowIndex);
	
	// QAbstractTableModel implementation
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	void sort(int columnIndex, Qt::SortOrder order = Qt::AscendingOrder) override;
private:
	void performSortByColumn(const CompositeColumn* column, Qt::SortOrder order, bool allowPassAndReverse);
	
private:
	QVariant computeCellContent(int bufferRowIndex, int columnIndex) const;
	int findOrderIndexForInsertedItem(int insertedItemBufferRowIndex);
};



#endif // COMPOSITE_TABLE_H
