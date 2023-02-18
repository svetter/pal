#ifndef COLUMN_H
#define COLUMN_H

#include <QSet>



enum DataType {
	integer, bit, varchar, date, time_
};


class Table;
class NormalTable;
class AssociativeTable;



struct WhatIfDeleteResult {
	const Table*		affectedTable;
	const NormalTable*	itemTable;
	QSet<int>			affectedRowIndices;
	
	WhatIfDeleteResult(const Table* affectedTable, const NormalTable* itemTable, QSet<int> affectedRowIndices);
};



class Column {
	QString			name;
	QString			uiName;
	DataType		type;
	bool			primaryKey;
	const Column*	foreignKey;
	bool			nullable;
	const Table*	inTable;
	
public:
	Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, const Column* foreignKey, const Table* inTable);
	
	QString			getName() const;
	QString			getUIName() const;
	DataType		getType() const;
	bool			isPrimaryKey() const;
	bool			isForeignKey() const;
	const Column*	getReferencedForeignColumn() const;
	bool			isNullable() const;
	const Table*	getTable() const;
	
	int getIndex() const;
	
	friend class NormalTable;
	friend class AssociativeTable;
};



QString getColumnListStringOf(QList<const Column*> columns);



QString getTranslatedWhatIfDeleteResultDescription(const WhatIfDeleteResult& whatIfResult);
QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults);



#endif // COLUMN_H
