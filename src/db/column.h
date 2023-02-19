#ifndef COLUMN_H
#define COLUMN_H

#include <QSet>

class Table;
class NormalTable;
class AssociativeTable;
struct WhatIfDeleteResult;



enum DataType {
	integer, bit, varchar, date, time_
};



class Column {
public:
	const QString		name;
	const QString		uiName;
	const DataType		type;
	const bool			primaryKey;
	const Column* const	foreignKey;
	const bool			nullable;
	const Table* const	table;
	
	Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, const Column* foreignKey, const Table* table);
	
	bool			isPrimaryKey() const;
	bool			isForeignKey() const;
	const Column*	getReferencedForeignColumn() const;
	int				getIndex() const;
};



QString getColumnListStringOf(QList<const Column*> columns);



QString getTranslatedWhatIfDeleteResultDescription(const WhatIfDeleteResult& whatIfResult);
QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults);



#endif // COLUMN_H
