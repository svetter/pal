#ifndef COMPOSITE_TABLE_H
#define COMPOSITE_TABLE_H

#include "src/comp_tables/composite_column.h"
#include "src/db/database.h"

#include <QAbstractTableModel>
#include <QProgressDialog>



class CompositeTable : public QAbstractTableModel {
	const NormalTable* baseTable;
	Database* db;
	
	QList<const CompositeColumn*> columns;
	
	QList<QList<QVariant>*> buffer;
	QList<int> bufferOrder;
	QPair<const CompositeColumn*, Qt::SortOrder> currentSorting;
	
public:
	const QString name;
	
protected:
	CompositeTable(Database* db, NormalTable* baseTable);
public:
	~CompositeTable();
	
protected:
	void addColumn(const CompositeColumn* column);
public:
	int getIndexOf(const CompositeColumn* column) const;
	const NormalTable* getBaseTable() const;
	
	void initBuffer(QProgressDialog* progressDialog);
	void resetBuffer();
	int getBufferRowForViewRow(int viewRowIndex) const;
	int findCurrentViewRowIndex(int bufferRowIndex) const;
	
	virtual QPair<const CompositeColumn*, Qt::SortOrder> getDefaultSorting() const = 0;
	
public:
	// Change annunciation
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
	QVariant computeCellContent(int bufferRowIndex, int columnIndex) const;
	int findOrderIndexForInsertedItem(int insertedItemBufferRowIndex);
};



#endif // COMPOSITE_TABLE_H
