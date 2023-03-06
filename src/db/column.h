#ifndef COLUMN_H
#define COLUMN_H

#include <QSet>

class Table;
struct WhatIfDeleteResult;
class CompositeColumn;



enum DataType {
	integer, bit, varchar, date, time_
};



class Column {
	QSet<const CompositeColumn*> changeListeners;
	
public:
	const QString		name;
	const QString		uiName;
	const DataType		type;
	const bool			primaryKey;
	Column* const		foreignKey;
	const bool			nullable;
	const Table* const	table;
	
	Column(QString name, QString uiName, DataType type, bool nullable, bool primaryKey, Column* foreignKey, const Table* table);
	
	bool	isPrimaryKey() const;
	bool	isForeignKey() const;
	bool	isKey() const;
	Column*	getReferencedForeignColumn() const;
	int		getIndex() const;
	
	QString getSqlSpecificationString() const;
	
	void registerChangeListener(const CompositeColumn* compositeColumn);
	QSet<const CompositeColumn*> getChangeListeners() const;
};



QString getColumnListStringOf(QList<const Column*> columns);



QString getTranslatedWhatIfDeleteResultDescription(const WhatIfDeleteResult& whatIfResult);
QString getTranslatedWhatIfDeleteResultDescription(const QList<WhatIfDeleteResult>& whatIfResults);



#endif // COLUMN_H
