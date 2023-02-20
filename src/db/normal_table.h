#ifndef NORMAL_TABLE_H
#define NORMAL_TABLE_H

#include "table.h"
#include "src/data/item_id.h"

#include <QString>
#include <QList>



class NormalTable : public Table {
public:
	const Column* primaryKeyColumn;
	
	NormalTable(QString name, QString uiName, QString primaryKeyColumnName);
	virtual ~NormalTable();
	
public:
	// Buffer access
	int getBufferIndexForPrimaryKey(ValidItemID primaryKey) const;
	
	// Modifications (passthrough)
	int addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void updateCell(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data);
	void updateRow(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data);
	void removeRow(QWidget* parent, const ValidItemID primaryKey);
	
	// Translation strings
	virtual QString getNoneString() const = 0;
	virtual QString getItemNameSingularLowercase() const = 0;
	virtual QString getItemNamePluralLowercase() const = 0;
	
	// QAbstractItemModel implementation (completes implementation in Table)
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
};



#endif // NORMAL_TABLE_H
