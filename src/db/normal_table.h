#ifndef NORMAL_TABLE_H
#define NORMAL_TABLE_H

#include "table.h"

#include <QString>
#include <QList>
#include <QAbstractTableModel>



class NormalTable : public Table, public QAbstractItemModel {
	QString noneString;
	
	Column*			primaryKeyColumn;
	QList<Column*>	nonPrimaryColumns;
	
public:
	static const int PrimaryKeyRole;
	
	NormalTable(QString name, QString itemNameSingularLowercase, QString uiName, QString noneString);
	~NormalTable();
	
	void addColumn(Column* column);
	
	// Getters
	QList<Column*> getColumnList() const override;
	int getNumberOfColumns() const override;
	Column* getPrimaryKeyColumn() const;
	int getNumberOfNonPrimaryKeyColumns() const;
	QList<Column*> getNonPrimaryKeyColumnList() const;
	Column* getColumnByIndex(int index) const;
	int getBufferIndexForPrimaryKey(int primaryKey) const;
	
	int getNumberOfEntries(QWidget* parent);
	
	// Modifications
	int addRow(QList<QVariant>& data);
	
	// QAbstractItemModel implementation
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex getNormalRootModelIndex() const;
	QModelIndex getNullableRootModelIndex() const;
};



#endif // NORMAL_TABLE_H
