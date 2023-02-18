#ifndef NORMAL_TABLE_H
#define NORMAL_TABLE_H

#include "table.h"

#include <QString>
#include <QList>
#include <QAbstractTableModel>



class NormalTable : public Table, public QAbstractItemModel {
	QString itemNameSingularLowercase;
	QString noneString;
	
	const Column*			primaryKeyColumn;
	QList<const Column*>	nonPrimaryColumns;
	
public:
	static const int PrimaryKeyRole;
	
	NormalTable(QString name, QString itemNameSingularLowercase, QString uiName, QString noneString);
	~NormalTable();
	
	void addColumn(const Column* column);
	
	// Getters
	QList<const Column*> getColumnList() const override;
	int getNumberOfColumns() const override;
	const Column* getPrimaryKeyColumn() const;
	int getNumberOfNonPrimaryKeyColumns() const;
	QList<const Column*> getNonPrimaryKeyColumnList() const;
	QString getNonPrimaryKeyColumnListString() const;
	const Column* getColumnByIndex(int index) const;
	int getBufferIndexForPrimaryKey(int primaryKey) const;
	
	int getNumberOfRows() const;

	QString getItemNameSingularLowercase() const;
	
	// Modifications
	int addRow(QWidget* parent, const QList<QVariant>& data);
	void removeRow(QWidget* parent, int primaryKey);
	
	// QAbstractItemModel implementation	// TODO hide these except for friend classes?
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
