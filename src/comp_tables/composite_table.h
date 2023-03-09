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
	int rowBeingInserted;
	
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
	
public:
	// Change annunciation
	void beginInsertRow(int bufferRowIndex);
	void endInsertRow();
	void beginRemoveRow(int bufferRowIndex);
	void endRemoveRow();
	void announceChangesUnderColumn(int columnIndex);
	
	// QAbstractTableModel implementation
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
private:
	QVariant computeCellContent(int rowIndex, int columnIndex) const;
};



#endif // COMPOSITE_TABLE_H
