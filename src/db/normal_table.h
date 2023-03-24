#ifndef NORMAL_TABLE_H
#define NORMAL_TABLE_H

#include "table.h"
#include "src/data/item_id.h"

#include <QString>
#include <QList>



class NormalTable : public Table {
public:
	Column* const primaryKeyColumn;
	
	NormalTable(QString name, QString uiName, QString primaryKeyColumnName);
	virtual ~NormalTable();
	
public:
	// Buffer access
	int getBufferIndexForPrimaryKey(ValidItemID primaryKey) const;
	ValidItemID getPrimaryKeyAt(int bufferRowIndex) const;
	QList<QPair<ValidItemID, QVariant>> pairIDWith(const Column* column) const;
	
	// Modifications (passthrough)
	int addRow(QWidget* parent, const QList<const Column*>& columns, const QList<QVariant>& data);
	void updateCell(QWidget* parent, const ValidItemID primaryKey, const Column* column, const QVariant& data);
	void updateRow(QWidget* parent, const ValidItemID primaryKey, const QList<const Column*>& columns, const QList<QVariant>& data);
	// removeRow(...) doesn't exist to avoid row removal without reference search. Outside interface is Database::removeRow(...)
	
	// Translation strings
	virtual QString getNoneString() const = 0;
	virtual QString getItemNameSingularLowercase() const = 0;
	virtual QString getItemNamePluralLowercase() const = 0;
	
	// QAbstractItemModel implementation (completes implementation in Table)
	void multiData(const QModelIndex& index, QModelRoleDataSpan roleDataSpan) const override;
};



#endif // NORMAL_TABLE_H
