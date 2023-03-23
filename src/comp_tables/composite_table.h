#ifndef COMPOSITE_TABLE_H
#define COMPOSITE_TABLE_H

#include "composite_column.h"
#include "src/db/database.h"

#include <QAbstractTableModel>
#include <QProgressDialog>
#include <QTableView>



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
	QTableView* tableToAutoResizeAfterCompute;
	
public:
	const QString name;
	
protected:
	static inline QString noSuffix = QString();
	static inline QString mSuffix = " m";
	
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
	void initBuffer(QProgressDialog* progressDialog, bool deferCompute = false, QTableView* tableToAutoResizeAfterCompute = nullptr);
	void rebuildOrderBuffer(bool skipRepopulate = false);
	int getNumberOfCellsToUpdate() const;
	void updateBuffer(QProgressDialog* progressDialog);
	void resetBuffer();
	int getBufferRowIndexForViewRow(int viewRowIndex) const;
	int findCurrentViewRowIndex(int bufferRowIndex) const;
	
	QVariant getRawValue(int bufferRowIndex, int columnIndex) const;
	QVariant getRawValue(int bufferRowIndex, const CompositeColumn* column) const;
	QVariant getFormattedValue(int bufferRowIndex, int columnIndex) const;
	QVariant getFormattedValue(int bufferRowIndex, const CompositeColumn* column) const;
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const = 0;
	QPair<const CompositeColumn*, Qt::SortOrder> getCurrentSorting() const;
	
	void setInitialFilters(QSet<Filter> filters);
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
};



#endif // COMPOSITE_TABLE_H
