#ifndef FILTER_H
#define FILTER_H

#include "src/db/breadcrumbs.h"
#include "src/db/normal_table.h"
#include "src/filters/filter_fold_op.h"

class FilterBox;



class Filter
{
public:
	const DataType type;
	const DataType sourceType;
	const NormalTable& tableToFilter;
	const Column& columnToFilterBy;
	const FilterFoldOp foldOp;
protected:
	const Breadcrumbs crumbs;
public:
	const bool isLocalFilter;
	const bool singleValuePerRow;
	
	const QString name;
	
private:
	bool enabled;
	bool inverted;
	
protected:
	Filter(DataType type, const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name);
public:
	virtual ~Filter();
	
public:
	bool isEnabled() const;
	bool isInverted() const;
	void setEnabled(bool enabled);
	void setInverted(bool inverted);
	
	void applyToOrderBuffer(ViewOrderBuffer& viewOrderBuffer) const;
private:
	QVariant getRawRowValue(const BufferRowIndex filteredTableBufferRow) const;
protected:
	virtual bool evaluate(const QVariant& rawRowValue) const = 0;
	
public:
	virtual FilterBox* createFilterBox(QWidget* parent) = 0;
	
	
	// Encoding & Decoding
	static QString encodeToString(QList<const Filter*> filters);
private:
	QString encodeSingleFilterToString() const;
public:
	static QList<Filter*> decodeFromString(const QString& encoded, Database& db);
private:
	static Filter* decodeSingleFilterFromString(QString& restOfString, Database& db);
protected:
	virtual QStringList encodeTypeSpecific() const = 0;
	
	// Encoding helpers
	static QString encodeInt	(const QString& paramName, int value);
	static QString encodeID		(const QString& paramName, const ItemID& value);
	static QString encodeBool	(const QString& paramName, bool value);
	static QString encodeString	(const QString& paramName, const QString& value);
	static QString encodeDate	(const QString& paramName, const QDate& value);
	static QString encodeTime	(const QString& paramName, const QTime& value);
	
	// Decoding helpers
	static DataType				decodeHeader		(QString& restOfString, bool& ok);
	static const NormalTable*	decodeTableIdentity	(QString& restOfString, const QString& tableNameParamName, bool& ok, Database& db);
	static const Column*		decodeColumnIdentity(QString& restOfString, const QString& tableNameParamName, const QString& columnNameParamName, bool& ok, Database& db);
	static int					decodeInt			(QString& restOfString, const QString& paramName, bool& ok, bool lastParam = false);
	static ItemID				decodeID			(QString& restOfString, const QString& paramName, bool& ok, bool lastParam = false);
	static bool					decodeBool			(QString& restOfString, const QString& paramName, bool& ok, bool lastParam = false);
	static QString				decodeString		(QString& restOfString, const QString& paramName, bool& ok, bool lastParam = false);
	static QDate				decodeDate			(QString& restOfString, const QString& paramName, bool& ok, bool lastParam = false);
	static QTime				decodeTime			(QString& restOfString, const QString& paramName, bool& ok, bool lastParam = false);
};



#endif // FILTER_H
