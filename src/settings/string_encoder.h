#ifndef STRING_ENCODER_H
#define STRING_ENCODER_H

#include "src/data/item_id.h"
#include "src/db/db_data_type.h"
#include "src/db/normal_table.h"
#include "src/comp_tables/comp_column_type.h"

#include <QString>



class StringEncoder
{
public:
	// Encoding helpers
	static QString encodeInt	(const QString& paramName, int value);
	static QString encodeID		(const QString& paramName, const ItemID& value);
	static QString encodeBool	(const QString& paramName, bool value);
	static QString encodeString	(const QString& paramName, const QString& value);
	static QString encodeDate	(const QString& paramName, const QDate& value);
	static QString encodeTime	(const QString& paramName, const QTime& value);
	
	// Decoding helpers
	template<typename T>
	static T					decodeHeader		(QString& restOfEncoding, const QString& encodedTypeSuffix, std::function<T(const QString&)> decodeType, bool& ok);
	static const NormalTable*	decodeTableIdentity	(QString& restOfEncoding, const QString& tableNameParamName, Database& db, bool& ok);
	static const Column*		decodeColumnIdentity(QString& restOfEncoding, const QString& tableNameParamName, const QString& columnNameParamName, Database& db, bool& ok, bool lastParam = false);
	static int					decodeInt			(QString& restOfEncoding, const QString& paramName, bool& ok, bool lastParam = false);
	static ItemID				decodeID			(QString& restOfEncoding, const QString& paramName, bool& ok, bool lastParam = false);
	static bool					decodeBool			(QString& restOfEncoding, const QString& paramName, bool& ok, bool lastParam = false);
	static QString				decodeString		(QString& restOfEncoding, const QString& paramName, bool& ok, bool lastParam = false);
	static QDate				decodeDate			(QString& restOfEncoding, const QString& paramName, bool& ok, bool lastParam = false);
	static QTime				decodeTime			(QString& restOfEncoding, const QString& paramName, bool& ok, bool lastParam = false);
};



#endif // STRING_ENCODER_H
