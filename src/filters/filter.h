#ifndef FILTER_H
#define FILTER_H

#include "src/db/normal_table.h"

class FilterBox;

using std::unique_ptr, std::make_unique;



class Filter
{
public:
	const DataType type;
	const NormalTable& tableToFilter;
	const Column& columnToFilterBy;
	
	const QString name;
	
private:
	bool enabled;
	bool inverted;
	
protected:
	Filter(DataType type, const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
public:
	bool isEnabled() const;
	bool isInverted() const;
	void setEnabled(bool enabled);
	void setInverted(bool inverted);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const = 0;
	

	// Encoding & Decoding
	QString encodeToString() const;
	static unique_ptr<Filter> decodeFromString(const QString& encoded, Database& db);
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
