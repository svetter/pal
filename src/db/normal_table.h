#ifndef NORMAL_TABLE_H
#define NORMAL_TABLE_H

#include "table.h"
#include "src/data/item_id.h"

#include <QString>
#include <QList>



class NormalTable : public Table {
	const Column*			primaryKeyColumn;
	QList<const Column*>	nonPrimaryColumns;
	
public:
	NormalTable(QString name, QString uiName, QString primaryKeyColumnName);
	virtual ~NormalTable();
	
	void addColumn(const Column* column);
	
	// Getters
	QList<const Column*> getColumnList() const override;
	QList<const Column*> getPrimaryKeyColumnList() const override;
	int getNumberOfColumns() const override;
	const Column* getPrimaryKeyColumn() const;
	int getNumberOfNonPrimaryKeyColumns() const;
	QList<const Column*> getNonPrimaryKeyColumnList() const;
	QString getNonPrimaryKeyColumnListString() const;
	int getBufferIndexForPrimaryKey(ValidItemID primaryKey) const;
	
	int getNumberOfRows() const;
	
	virtual QString getNoneString() const = 0;
	virtual QString getItemNameSingularLowercase() const = 0;
	virtual QString getItemNamePluralLowercase() const = 0;
	
	// Modifications
	int addRow(QWidget* parent, const QList<QVariant>& data);
	void updateCell(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data);
	void updateRow(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const ValidItemID primaryKey);
	
	// QAbstractItemModel implementation
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
	
	static const int PrimaryKeyRole;
	QModelIndex getNullableRootModelIndex() const;
};



#endif // NORMAL_TABLE_H
