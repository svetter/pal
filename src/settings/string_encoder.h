#ifndef STRING_ENCODER_H
#define STRING_ENCODER_H

#include "src/data/item_id.h"
#include "src/db/normal_table.h"

#include <QString>

class CompositeColumn;
class CompositeTable;
class ItemTypesHandler;



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
	static T						decodeHeader			(QString& restOfEncoding, const QString& encodedTypeSuffix, std::function<T(const QString&)> decodeType, bool& ok);
	static const NormalTable*		decodeTableIdentity		(QString& restOfEncoding, const QString& tableNameParamName, const Database& db, bool& ok);
	static const Column*			decodeColumnIdentity	(QString& restOfEncoding, const QString& tableNameParamName, const QString& columnNameParamName, const Database& db, bool& ok);
	static const CompositeTable*	decodeCompTableIdentity	(QString& restOfEncoding, const QString& tableNameParamName, const ItemTypesHandler& typesHandler, bool& ok);
	static const CompositeColumn*	decodeCompColumnIdentity(QString& restOfEncoding, const QString& tableNameParamName, const QString& columnNameParamName, const ItemTypesHandler& typesHandler, bool& ok);
	static int						decodeInt				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static ItemID					decodeID				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static bool						decodeBool				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static QString					decodeString			(QString& restOfEncoding, const QString& paramName, bool& ok);
	static QDate					decodeDate				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static QTime					decodeTime				(QString& restOfEncoding, const QString& paramName, bool& ok);
};



#endif // STRING_ENCODER_H
